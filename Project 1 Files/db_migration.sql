PRAGMA foreign_keys = ON;
CREATE TABLE IF NOT EXISTS courses(
  course_num  TEXT PRIMARY KEY,
  course_name TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS prereqs(
  course_num TEXT NOT NULL,
  prereq     TEXT NOT NULL,
  PRIMARY KEY(course_num, prereq),
  FOREIGN KEY(course_num) REFERENCES courses(course_num) ON DELETE CASCADE
);
