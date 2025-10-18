# Course Planner – Algorithms & Database Enhancements Pseudocode

This document summarizes the new algorithmic and database functionality in pseudocode.

---

## Part A — Algorithms & Data Structures

### A1) Safer CSV Parsing + Normalization

```
function loadCourses(csvPath, bst, outVector)
    open file at csvPath
    outVector.clear()
    lineNo = 0
    for each line in file (using getline)
        lineNo++
        strip UTF‑8 BOM if present on first line
        trim whitespace on both ends
        if line is empty or starts with '#': continue

        fields = split line by ','
        trim each field

        if fields.size < 2:
            warn "malformed line lineNo" and continue

        course.id    = UPPERCASE(TRIM(fields[0]))
        course.name  = fields[1]
        course.preReqs = empty list

        seen = empty set
        for i from 2 to fields.size-1:
            pre = UPPERCASE(TRIM(fields[i]))
            if pre not empty and pre not in seen:
                add pre to seen
                append pre to course.preReqs

        append course to outVector
        bst.Insert(course)
    close file
```

**Notes**
- Uses `getline` loop (no `while(!eof())`).
- Normalizes IDs to a canonical form (uppercase, trimmed).
- De‑duplicates per‑course prerequisites to avoid inflating indegree later.

---

### A2) Build Graph for Prerequisites

```
function buildGraph(courses):
    adj = map from string to list<string>   // edges: prereq -> dependent
    indegree = map from string to int       // counts of incoming edges

    // Initialize nodes with zero indegree
    for each course in courses:
        indegree[course.id] = 0

    warnedMissing = false
    for each course in courses:
        seen = empty set
        for each pre in course.preReqs:
            if pre already in seen: continue
            add pre to seen
            if pre not in indegree and not warnedMissing:
                print "Warning: prereq 'pre' not found in catalog"
                warnedMissing = true
            append course.id to adj[pre]
            indegree[course.id]++
            if pre not in indegree: indegree[pre] = 0  // ensure node exists

    return (adj, indegree)
```

---

### A3) Kahn’s Topological Sort (Valid Course Order)

```
function topoKahn(courses):
    (adj, indegree) = buildGraph(courses)

    q = queue of all nodes where indegree[node] == 0
    order = empty list

    while q not empty:
        u = pop front of q
        append u to order
        for each v in adj[u]:
            indegree[v]--
            if indegree[v] == 0:
                push v into q

    if order.size != number of nodes in indegree:
        return empty list  // cycle detected
    return order
```

---

### A4) Inline Graph View (Readable Console Lines)

```
function printInlineGraph(courses):
    table = map course.id -> set of direct prereqs

    for each course in courses:
        ensure table[course.id] exists (possibly empty)
        for each pre in course.preReqs:
            insert pre into table[course.id]

    sortedIds = sorted list of keys in table
    print "== Inline Prerequisite Graph =="
    for each id in sortedIds:
        if table[id] empty:
            print id + " <- (none)"
        else
            join prereqs with ", "
            print id + " <- " + joined_prereqs
```

---

## Part B — Database (SQLite) CRUD

### B1) Open Database and Migrate Schema

```
class CourseRepository
    has db connection pointer

    function open(path):
        if existing connection open: close it
        open SQLite database at path
        execute "PRAGMA foreign_keys = ON"
        call migrate()

    function migrate():
        execute SQL:
            CREATE TABLE IF NOT EXISTS courses(
              course_num  TEXT PRIMARY KEY,
              course_name TEXT NOT NULL
            );
        execute SQL:
            CREATE TABLE IF NOT EXISTS prereqs(
              course_num TEXT NOT NULL,
              prereq     TEXT NOT NULL,
              PRIMARY KEY(course_num, prereq),
              FOREIGN KEY(course_num) REFERENCES courses(course_num) ON DELETE CASCADE
            );
```

---

### B2) Upsert One Course (Name + Full Prereq Set, Atomically)

```
function upsertCourse(course):
    begin transaction
    execute parameterized SQL to insert course row:
        INSERT INTO courses(course_num, course_name) VALUES(?, ?)
        ON CONFLICT(course_num) DO UPDATE SET course_name=excluded.course_name;

    execute parameterized SQL to delete old prereqs for course_num
    for each pre in course.preReqs:
        execute parameterized SQL to INSERT OR IGNORE a (course_num, prereq) row
    commit
```

---

### B3) Import a Batch (CSV → DB) Efficiently

```
function importBatch(listOfCourses):
    begin transaction

    prepare 3 statements once:
        S1: upsert course row
        S2: delete prereqs for a given course
        S3: insert-or-ignore a single prereq row

    for each course in listOfCourses:
        bind and run S1 with (course.id, course.name)
        bind and run S2 with (course.id)
        for each pre in course.preReqs:
            bind and run S3 with (course.id, pre)

    finalize all statements
    commit
```

(One transaction and reused statements avoids “transaction-within-transaction” errors and speeds up large imports.)

---

### B4) Read Operations

```
function getCourse(id) -> (found, course):
    query base row by id
    if not found: return (false, empty course)
    query prereqs ordered by prereq
    attach prereqs to course
    return (true, course)
```

```
function getAllCourses() -> list<Course>:
    query all base rows ordered by course_num
    store in a list
    query all prereqs ordered by (course_num, prereq)
    for each prereq row, attach to the matching course in memory
    return list
```

---

### B5) Update & Delete

```
function updateCourseName(id, newName) -> bool:
    run UPDATE with parameters
    return true if a row was changed, else false
```

```
function deleteCourse(id) -> bool:
    run DELETE with parameter
    return true if a row was deleted, else false
```

---

## Part C — Menu Wiring 

```
Menu option 1: Load from CSV
    ask for filename; append ".csv" if missing
    if file exists:
        loadCourses(file, bst, allCourses)
        repo.open("courses.db") 
        repo.importBatch(allCourses)

Menu option 6: Load from database
    repo.open("courses.db")
    rows = repo.getAllCourses()
    allCourses = rows
    rebuild bst from rows

Menu option 7: Upsert a course in database
    read id, name, comma-separated prereqs
    normalize every id
    repo.open("courses.db")
    repo.upsertCourse(course)

Menu option 8: Delete a course in database
    read id; normalize
    repo.open("courses.db")
    repo.deleteCourse(id)
```
