#pragma once
//=============================================================================
// Database.h                                                 *New Header File*
//-----------------------------------------------------------------------------
// This file defines the CourseRepository class, which provides a CRUD layer
// (Create, Read, Update, Delete) for Course data stored in an SQLite database.
//
// Why?
// - Adds persistence to the Course Planner (data is saved between runs)
// - Supports clean database operations (instead of only CSV/BST in memory)
//
// Usage:
//   CourseRepository repo;
//   repo.open("courses.db");
//   repo.upsertCourse(course);
//   repo.getCourse("CS200", outCourse);
//   repo.deleteCourse("CS200");
//
// Requires: sqlite3 library (sqlite3.h, sqlite3.lib/sqlite3.dll or sqlite3.c)
//=============================================================================

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>

#include "CourseModel.h"

// Forward declarations for SQLite types (avoids pulling sqlite3.h into header)
struct sqlite3;
struct sqlite3_stmt;

//-----------------------------------------------------------------------------
// CourseRepository
//-----------------------------------------------------------------------------
// Encapsulates all interaction with the SQLite database.
// - Hides raw SQL commands behind clean methods
// - Manages schema creation ("migration")
// - Wraps operations in transactions for consistency
//-----------------------------------------------------------------------------
class CourseRepository {
public:
    CourseRepository();
    ~CourseRepository();

    // Prevent accidental copying (one DB connection per repo)
    CourseRepository(const CourseRepository&) = delete;
    CourseRepository& operator=(const CourseRepository&) = delete;

    // Allow move semantics (can transfer ownership)
    CourseRepository(CourseRepository&& other) noexcept;
    CourseRepository& operator=(CourseRepository&& other) noexcept;

    // Open or create a database at the given path.
    // Also ensures required tables exist (via migrate()).
    void open(const std::string& path);

    // -------------------- CRUD OPERATIONS --------------------

    // Insert or update a course and its prerequisites (atomic "upsert").
    void upsertCourse(const Course& c);

    // Retrieve a single course by ID.
    // Returns true if found, false otherwise.
    bool getCourse(const std::string& id, Course& out);

    // Retrieve all courses (with prerequisites).
    std::vector<Course> getAllCourses();

    // Update only the course name. Returns false if course not found.
    bool updateCourseName(const std::string& id, const std::string& newName);

    // Replace the full prerequisite set for a course.
    bool setPrereqs(const std::string& id, const std::vector<std::string>& prereqs);

    // Delete a course (and its prereqs via ON DELETE CASCADE).
    bool deleteCourse(const std::string& id);

    // Insert multiple courses atomically (batch import, e.g., after CSV load).
    void importBatch(const std::vector<Course>& batch);

    // Utility: run a function inside a transaction (auto rollback on error).
    void inTransaction(const std::function<void()>& cb);

private:
    sqlite3* db_ = nullptr; // Pointer to the SQLite database connection

    // -------------------- Internal helpers --------------------

    void close(); // Closes the DB connection
    void migrate(); // Creates tables if they don’t exist
    void exec(const std::string& sql); // Execute raw SQL (no results)

    void prepare(const std::string& sql, sqlite3_stmt** stmt); // Compile SQL
    void finalize(sqlite3_stmt* stmt); // Free prepared statement
    void bindText(sqlite3_stmt* stmt, int idx, const std::string& s); // Bind strings
    bool stepRow(sqlite3_stmt* stmt); // Step through results safely

    // Transaction helpers
    void begin();
    void commit();
    void rollback();
};