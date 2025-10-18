//=============================================================================
// Database.cpp                                                  *New C++ File*
//-----------------------------------------------------------------------------

#include "Database.h"
#include "sqlite/sqlite3.h"
#include <stdexcept>
#include <sstream>

//=============================================================================
// Helper: construct a std::runtime_error from an SQLite error
//=============================================================================
static inline std::runtime_error dberr(sqlite3* db, const char* where) {
    std::ostringstream oss;
    oss << "SQLite error in " << where << ": " << sqlite3_errmsg(db);
    return std::runtime_error(oss.str());
}

//=============================================================================
// Lifecycle
//=============================================================================
CourseRepository::CourseRepository() {}
CourseRepository::~CourseRepository() { close(); }

// Move constructor
CourseRepository::CourseRepository(CourseRepository&& other) noexcept {
    db_ = other.db_;
    other.db_ = nullptr;
}

// Move assignment
CourseRepository& CourseRepository::operator=(CourseRepository&& other) noexcept {
    if (this != &other) {
        close();
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

//=============================================================================
// Connection management
//=============================================================================
void CourseRepository::open(const std::string& path) {
    close(); // Close any old connection

    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw dberr(db_, "sqlite3_open");
    }

    // Enforce foreign keys (disabled by default in SQLite)
    exec("PRAGMA foreign_keys = ON;");

    // Ensure schema exists
    migrate();
}

void CourseRepository::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

//=============================================================================
// Internal helpers
//=============================================================================
void CourseRepository::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("SQLite exec error: " + msg);
    }
}

void CourseRepository::prepare(const std::string& sql, sqlite3_stmt** stmt) {
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, stmt, nullptr) != SQLITE_OK) {
        throw dberr(db_, "sqlite3_prepare_v2");
    }
}

void CourseRepository::finalize(sqlite3_stmt* stmt) {
    sqlite3_finalize(stmt);
}

void CourseRepository::bindText(sqlite3_stmt* stmt, int idx, const std::string& s) {
    if (sqlite3_bind_text(stmt, idx, s.c_str(), (int)s.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
        throw dberr(db_, "sqlite3_bind_text");
    }
}

bool CourseRepository::stepRow(sqlite3_stmt* stmt) {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw dberr(db_, "sqlite3_step");
}

// Transaction helpers
void CourseRepository::begin() { exec("BEGIN IMMEDIATE;"); }
void CourseRepository::commit() { exec("COMMIT;"); }
void CourseRepository::rollback() { exec("ROLLBACK;"); }

//=============================================================================
// Migration (creates schema if it does not exist)
//=============================================================================
void CourseRepository::migrate() {
    exec("CREATE TABLE IF NOT EXISTS courses("
        "  course_num TEXT PRIMARY KEY,"
        "  course_name TEXT NOT NULL"
        ");");

    exec("CREATE TABLE IF NOT EXISTS prereqs("
        "  course_num TEXT NOT NULL,"
        "  prereq     TEXT NOT NULL,"
        "  PRIMARY KEY(course_num, prereq),"
        "  FOREIGN KEY(course_num) REFERENCES courses(course_num) ON DELETE CASCADE"
        ");");
}

//=============================================================================
// Transactions
//=============================================================================
void CourseRepository::inTransaction(const std::function<void()>& cb) {
    begin();
    try {
        cb(); // Run user-provided code
        commit(); // If no exception, commit
    }
    catch (...) {
        rollback(); // On error, rollback
        throw;
    }
}

//=============================================================================
// CRUD operations
//=============================================================================
void CourseRepository::upsertCourse(const Course& c) {
    inTransaction([&] {
        // Insert or update the course row
        {
            sqlite3_stmt* st = nullptr;
            prepare("INSERT INTO courses(course_num, course_name) VALUES(?, ?) "
                "ON CONFLICT(course_num) DO UPDATE SET course_name=excluded.course_name;",
                &st);
            bindText(st, 1, c.courseNum);
            bindText(st, 2, c.courseName);
            stepRow(st); // Expect SQLITE_DONE
            finalize(st);
        }

        // Replace all prereqs for this course
        {
            sqlite3_stmt* del = nullptr;
            prepare("DELETE FROM prereqs WHERE course_num=?", &del);
            bindText(del, 1, c.courseNum);
            stepRow(del);
            finalize(del);

            if (!c.preReqs.empty()) {
                sqlite3_stmt* ins = nullptr;
                prepare("INSERT OR IGNORE INTO prereqs(course_num, prereq) VALUES(?, ?);", &ins);
                for (const auto& p : c.preReqs) {
                    sqlite3_reset(ins);
                    bindText(ins, 1, c.courseNum);
                    bindText(ins, 2, p);
                    stepRow(ins);
                }
                finalize(ins);
            }
        }
        });
}

bool CourseRepository::getCourse(const std::string& id, Course& out) {
    out = Course{}; // reset
    sqlite3_stmt* st = nullptr;

    // Fetch base row
    prepare("SELECT course_num, course_name FROM courses WHERE course_num=?", &st);
    bindText(st, 1, id);
    if (!stepRow(st)) { finalize(st); return false; }
    out.courseNum = reinterpret_cast<const char*>(sqlite3_column_text(st, 0));
    out.courseName = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
    finalize(st);

    // Fetch prereqs
    prepare("SELECT prereq FROM prereqs WHERE course_num=? ORDER BY prereq;", &st);
    bindText(st, 1, id);
    while (stepRow(st)) {
        out.preReqs.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(st, 0)));
    }
    finalize(st);

    return true;
}

std::vector<Course> CourseRepository::getAllCourses() {
    std::vector<Course> out;
    sqlite3_stmt* st = nullptr;

    // Fetch all base rows
    prepare("SELECT course_num, course_name FROM courses ORDER BY course_num;", &st);
    while (stepRow(st)) {
        Course c;
        c.courseNum = reinterpret_cast<const char*>(sqlite3_column_text(st, 0));
        c.courseName = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
        out.push_back(std::move(c));
    }
    finalize(st);

    // Fetch prereqs and attach to matching courses
    prepare("SELECT course_num, prereq FROM prereqs ORDER BY course_num, prereq;", &st);
    while (stepRow(st)) {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(st, 0));
        std::string pre = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
        for (auto& c : out) if (c.courseNum == id) { c.preReqs.push_back(pre); break; }
    }
    finalize(st);

    return out;
}

bool CourseRepository::updateCourseName(const std::string& id, const std::string& newName) {
    sqlite3_stmt* st = nullptr;
    prepare("UPDATE courses SET course_name=? WHERE course_num=?;", &st);
    bindText(st, 1, newName);
    bindText(st, 2, id);
    int rc = sqlite3_step(st);
    bool any = (rc == SQLITE_DONE && sqlite3_changes(db_) > 0);
    finalize(st);
    return any;
}

bool CourseRepository::setPrereqs(const std::string& id, const std::vector<std::string>& prereqs) {
    inTransaction([&] {
        // Delete old prereqs
        sqlite3_stmt* del = nullptr;
        prepare("DELETE FROM prereqs WHERE course_num=?", &del);
        bindText(del, 1, id);
        stepRow(del);
        finalize(del);

        // Insert new prereqs
        sqlite3_stmt* ins = nullptr;
        prepare("INSERT OR IGNORE INTO prereqs(course_num, prereq) VALUES(?, ?);", &ins);
        for (const auto& p : prereqs) {
            sqlite3_reset(ins);
            bindText(ins, 1, id);
            bindText(ins, 2, p);
            stepRow(ins);
        }
        finalize(ins);
        });
    return true;
}

bool CourseRepository::deleteCourse(const std::string& id) {
    sqlite3_stmt* st = nullptr;
    prepare("DELETE FROM courses WHERE course_num=?;", &st);
    bindText(st, 1, id);
    int rc = sqlite3_step(st);
    bool any = (rc == SQLITE_DONE && sqlite3_changes(db_) > 0);
    finalize(st);
    return any;
}

void CourseRepository::importBatch(const std::vector<Course>& batch) {
    // Single transaction for the whole import
    begin();
    sqlite3_stmt* insCourse = nullptr;
    sqlite3_stmt* delPre = nullptr;
    sqlite3_stmt* insPre = nullptr;
    try {
        // Upsert course row (course_num, course_name)
        prepare(
            "INSERT INTO courses(course_num, course_name) VALUES(?, ?) "
            "ON CONFLICT(course_num) DO UPDATE SET course_name=excluded.course_name;",
            &insCourse
        );

        // Delete all prereqs for a course (we'll replace them)
        prepare("DELETE FROM prereqs WHERE course_num=?;", &delPre);

        // Insert (dedup) prerequisites
        prepare("INSERT OR IGNORE INTO prereqs(course_num, prereq) VALUES(?, ?);", &insPre);

        for (const auto& c : batch) {
            // upsert course
            sqlite3_reset(insCourse);
            bindText(insCourse, 1, c.courseNum);
            bindText(insCourse, 2, c.courseName);
            stepRow(insCourse); // expect DONE

            // delete old prereqs
            sqlite3_reset(delPre);
            bindText(delPre, 1, c.courseNum);
            stepRow(delPre);

            // insert new prereqs
            for (const auto& p : c.preReqs) {
                sqlite3_reset(insPre);
                bindText(insPre, 1, c.courseNum);
                bindText(insPre, 2, p);
                stepRow(insPre);
            }
        }

        finalize(insCourse);
        finalize(delPre);
        finalize(insPre);

        commit();
    }
    catch (...) {
        if (insCourse) finalize(insCourse);
        if (delPre)    finalize(delPre);
        if (insPre)    finalize(insPre);
        rollback();
        throw;
    }
}