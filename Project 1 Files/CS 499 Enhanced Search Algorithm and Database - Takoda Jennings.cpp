//============================================================================
// Name        : CS 499 Enhanced Search Algorithm and Database - Takoda Jennings.cpp
// Author      : Takoda Jennings
// Version     : 3.0
// Description : Search Algorithms in C++
//============================================================================

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <algorithm>
#include <cctype>
#include <sstream>

#include "Database.h"
#include "CSVparser.hpp"
#include "CourseModel.h"

using namespace std;

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

// structure for BST node
struct Node {
    Course course;
    Node* left;
    Node* right;

    // default constructor
    Node() {
        left = nullptr;
        right = nullptr;
    }

    // initialize with a course
    Node(Course aCourse) :
        Node() {
        this->course = aCourse;
    }
};

//============================================================================
// Binary Search Tree class definition
//============================================================================

class CourseBST {
private:
    Node* root;

    void addNode(Node* node, Course course);
    void printSampleSchedule(Node* node);
    void printCourseInformation(Node* current, const std::string& courseNum);

public:
    CourseBST();
    virtual ~CourseBST();
    void DeleteRecursive(Node* node);
    void Insert(Course course);
    int NumPrerequisiteCourses(Course course);
    void PrintSampleSchedule();
    void PrintCourseInformation(string courseNum);
};

// constructor to initialize root
CourseBST::CourseBST() {
    root = nullptr;
}

// pass root to PrintSampleSchedule method
void CourseBST::PrintSampleSchedule() {
    this->printSampleSchedule(root);
}

// pass root and course number to PrintCourseInformation method
void CourseBST::PrintCourseInformation(string courseNum) {
    this->printCourseInformation(root, courseNum);
}

// decontructor for BST
CourseBST::~CourseBST() {
    DeleteRecursive(root);
}

// method to delete node using deconstuctor
void CourseBST::DeleteRecursive(Node* node) {
    if (node) {
        DeleteRecursive(node->left);
        DeleteRecursive(node->right);
        delete node;
    }
}

// loop for counting and returning prerequisite courses
int CourseBST::NumPrerequisiteCourses(Course course) {
    int count = 0;
    for (std::size_t i = 0; i < course.preReqs.size(); ++i) {
        if (course.preReqs.at(i).length() > 0) {
            count++;
        }
    }
    return count;
}

// using the passed root, print schedule in alphabetical order 
void CourseBST::printSampleSchedule(Node* node) {
    if (node != nullptr) {
        printSampleSchedule(node->left);
        cout << node->course.courseNum << ", " << node->course.courseName << endl;
        printSampleSchedule(node->right);
    }
    return;
}

// method to insert node into BST
void CourseBST::Insert(Course course) {
    // if root empty, add node as the root
    if (root == nullptr) {
        root = new Node(course);
    }
    // else, add to BST
    else {
        this->addNode(root, course);
    }
}

// adds nodes based on if less than or greater than previous node
void CourseBST::addNode(Node* node, Course course) {
    // if current course's number is less than the current node's course number
    if (node->course.courseNum.compare(course.courseNum) > 0) {
        if (node->left == nullptr)
            node->left = new Node(course);
        else
            this->addNode(node->left, course);
    }
    // else current course number is equal or greater than the current node's course number
    else {
        if (node->right == nullptr)
            node->right = new Node(course);
        else
            this->addNode(node->right, course);
    }
}

// Trim whitespace (both ends)
static inline std::string trim(std::string s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

// Uppercase (useful to normalize course IDs)
static inline std::string upper(std::string s) {
    for (char& ch : s) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

// Strip UTF-8 BOM at start of string, if present
static inline void stripBOM(std::string& s) {
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
        s.erase(0, 3);
    }
}

// Normalize a course ID for consistent storage/lookup
static inline std::string normalizeId(std::string s) {
    stripBOM(s);
    s = trim(s);
    s = upper(s);
    return s;
}

// Split a CSV line on commas (no quotes handling needed for your dataset)
static inline std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> out;
    std::string field;
    std::stringstream ss(line);
    while (std::getline(ss, field, ',')) out.push_back(field);
    // If the line ends with a comma, add a final empty field
    if (!line.empty() && line.back() == ',') out.push_back("");
    return out;
}

// searches for a course and prints out current course and its prerequisite(s) else prints error 
void CourseBST::printCourseInformation(Node* current, const std::string& courseNum) {
    // Early guard eliminates C6011 ("dereferencing NULL pointer 'current'")
    if (current == nullptr) {
        std::cout << "Course " << courseNum << " not found." << std::endl;
        return;
    }

    // Normalize the input course number to uppercase
    std::string target = upper(courseNum);

    while (current != nullptr) {
        // Compare once and branch on the sign
        int cmp = current->course.courseNum.compare(courseNum);

        if (cmp == 0) {
            std::cout << "\n" << current->course.courseNum
                << ", " << current->course.courseName << std::endl;

            const auto& prereqs = current->course.preReqs;
            std::cout << "Prerequisite(s): ";
            if (prereqs.empty()) {
                std::cout << "(none)\n";
            }
            else {
                for (std::size_t i = 0; i < prereqs.size(); ++i) {
                    std::cout << prereqs[i];
                    if (i + 1 != prereqs.size()) std::cout << ", ";
                }
                std::cout << "\n";
            }
            return;
        }

        // Traverse left or right
        if (cmp > 0) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }

    std::cout << "Course " << courseNum << " not found." << std::endl;
}

// Load courses using CSVParser
void loadCourses(std::string csvPath, CourseBST* coursesBST, std::vector<Course>& outCourses) {
    if (!coursesBST) {
        std::cout << "Internal error: coursesBST is null. Aborting load.\n";
        return;
    }

    std::ifstream file(csvPath);
    if (!file) {
        std::cout << "Error: could not open '" << csvPath << "'.\n";
        return;
    }

    outCourses.clear();
    std::string line;
    std::size_t lineNo = 0;

    while (std::getline(file, line)) {
        ++lineNo;
        stripBOM(line); // handle BOM if it's on the first line
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        auto fields = splitCSV(line);
        for (auto& f : fields) f = trim(f);

        if (fields.size() < 2) {
            std::cout << "Warning: line " << lineNo << " has fewer than 2 fields; skipping.\n";
            continue;
        }

        Course course;
        course.courseNum = normalizeId(fields[0]); // <— normalized ID
        course.courseName = fields[1];

        std::unordered_set<std::string> seen;
        for (std::size_t i = 2; i < fields.size(); ++i) {
            auto id = normalizeId(fields[i]); // <— normalized prereq
            if (!id.empty() && seen.insert(id).second) {
                course.preReqs.push_back(id);
            }
        }

        outCourses.push_back(course);
        coursesBST->Insert(course); // BST stores normalized key
    }
}

//============================================================================
// Graph helpers + Kahn topological sort                        
//============================================================================

// Build adjacency and indegree from a flat list of Courses. 
// 
// Input: courses - flat list of Course {courseNum, name, preReqs}
// Output: adj[pre] - vector of courses that directly depend on 'pre'
//         indegree[course] - number of incoming edges for 'course'
// Notes:
//   * Inserts every known course into an 'indegree' so nodes with no prereqs appear.
//   * DeDuplicates prerequisites per course to avoid inflating indegree.
//   * Warns once if a prerequisite isn't present in the catalog.
static void buildGraph(const std::vector<Course>& courses,
    std::unordered_map<std::string, std::vector<std::string>>& adj,
    std::unordered_map<std::string, int>& indegree) {
    adj.clear(); indegree.clear();

    std::unordered_set<std::string> known;
    known.reserve(courses.size());
    for (const auto& c : courses) { known.insert(c.courseNum); indegree[c.courseNum] += 0; }

    bool warnedMissing = false; // avoids noisy output
    for (const auto& c : courses) {
        std::unordered_set<std::string> seen;
        for (const auto& pre : c.preReqs) {
            if (!seen.insert(pre).second) continue; // skips duplicate prereq
            if (!known.count(pre) && !warnedMissing) {
                std::cout << "Warning: prerequisite '" << pre << "' not found in catalog.\n";
                warnedMissing = true;
            }
            adj[pre].push_back(c.courseNum);
            ++indegree[c.courseNum];
            if (!indegree.count(pre)) indegree[pre] = 0; // ensures node exists
        }
    }
}

// Topological sort via Kahn's algorithm.
// 
// Returns: a vector of course IDs in a valid order, or empty if a cycle is detected.
// Invariants:
//   * The indegree of each node is computed from the graph at the start of the process
//   * A queue holds all current "ready" nodes (indegree == 0).
//   * Each processed node decrements the indegree of its neighbors.
//   * If, at the end, not all nodes were processed, the graph contains a cycle.
// 
// Complexity: O(V+E)
static std::vector<std::string> topoKahn(const std::vector<Course>& courses) {
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> indegree;
    buildGraph(courses, adj, indegree);

    std::queue<std::string> q;
    for (auto& kv : indegree) if (kv.second == 0) q.push(kv.first);

    // Pre-allocate to avoid reallocations; improves cache-friendliness on large catalogs.
    std::vector<std::string> order;
    order.reserve(indegree.size());

    while (!q.empty()) {
        auto u = q.front(); q.pop();
        order.push_back(u);
        for (auto& v : adj[u]) {
            if (--indegree[v] == 0) q.push(v);
        }
    }
    if (order.size() != indegree.size()) return {}; // cycle
    return order;
}

// Inline graph printer: each line shows a course and its direct prerequisites.
// Example:
//   CS300 <- CS200, MATH201
static void printInlineGraph(const std::vector<Course>& courses) {
    std::unordered_map<std::string, std::set<std::string>> prereqs;
    for (const auto& c : courses) {
        for (const auto& pre : c.preReqs) prereqs[c.courseNum].insert(pre);
        if (!prereqs.count(c.courseNum)) prereqs[c.courseNum];
    }
    std::vector<std::string> keys; keys.reserve(prereqs.size());
    for (auto& kv : prereqs) keys.push_back(kv.first);
    std::sort(keys.begin(), keys.end());

    std::cout << "\nInline Prerequisite Graph:\n";
    for (const auto& k : keys) {
        std::cout << k << " <- ";
        bool first = true;
        for (const auto& pre : prereqs[k]) { if (!first) std::cout << ", "; std::cout << pre; first = false; }
        if (first) std::cout << "(none)";
        std::cout << "\n";
    }
}

/**
 * The one and only main() method
 */
int main(int argc, char* argv[]) {

    // Constructs a CourseBST object 
    CourseBST coursesBST;

    // Creates an empty vector of Course objects named allCourses.
    std::vector<Course> allCourses;

    // Creates a repository to store courses in the database
    CourseRepository repo;

    // process command line arguments
    string csvPath, courseId;
    switch (argc) {
    case 2: 
        csvPath = argv[1]; 
        break;
    case 3: 
        csvPath = argv[1]; 
        courseId = argv[2]; 
        break;
    default: 
        csvPath.clear(); 
        break;
    }

    cout << "\nWelcome to the course planner!\n" << endl;

    // display menu
    int choice = 0;
    while (choice != 9) {
        cout << "Menu:" << endl;
        cout << "  1. Load Data Stucture" << endl;
        cout << "  2. Print Course List" << endl;
        cout << "  3. Print course" << endl;
        cout << "  4. Topological order (Kahn's algorithm)" << endl;
        cout << "  5. Show inline prerequisite graph" << endl;
        cout << "  6. Load Data Structure from Database (SQLite)" << endl;
        cout << "  7. Upsert a Course in Database (insert/update)" << endl;
        cout << "  8. Delete a Course in Database" << endl;
        cout << "  9. Exit" << endl;
        cout << "\nWhat would you like to do? ";
        cin >> choice;
      
        switch (choice) {
            case 1: {
                // *Updated*
                // Load courses from csv and add to database 
                cout << "Enter the name of the course file to add to the database (e.g., Courses or Courses.csv): ";
                string csvPath;
                std::getline(cin >> ws, csvPath);

                // Append .csv if user omitted extension
                if (csvPath.find('.') == string::npos) {
                    csvPath += ".csv";
                }

                // Verify file exists before loading
                ifstream test(csvPath);
                if (!test) {
                    cout << "Error: could not open file '" << csvPath << "'." << endl;
                    break; // back to menu
                }

                loadCourses(csvPath, &coursesBST, allCourses);
                cout << "Courses loaded successfully from " << csvPath << "!" << endl;

                // After a successful CSV load
                try {
                    repo.open("courses.db"); // create/open DB once
                    repo.importBatch(allCourses); // atomic: upsert all rows + prereqs
                    std::cout << "Imported " << allCourses.size()
                        << " courses into the database.\n";
                }
                catch (const std::exception& e) {
                    std::cout << "DB error: " << e.what() << "\n";
                }

                break;
            }

            case 2: {
                // Print course list
                if (allCourses.empty()) {
                    cout << "Please load the course data first (option 1)." << endl;
                }
                else {
                    coursesBST.PrintSampleSchedule();
                }
                break;
            }

            case 3: {
                // Print a single course
                if (allCourses.empty()) {
                    cout << "Please load the course data first (option 1)." << endl;
                    break;
                }
                cout << "Enter course ID: ";
                std::string courseId;
                std::getline(cin >> ws, courseId);
                courseId = normalizeId(courseId); // Normalize input
                coursesBST.PrintCourseInformation(courseId);
                break;
            }

            case 4: {
                // Print a list in topological order (Kahn’s algorithm)
                if (allCourses.empty()) {
                    cout << "Please load the course data first (option 1)." << endl;
                    break;
                }
                auto order = topoKahn(allCourses);
                if (order.empty()) {
                     cout << "Cycle detected: graph is not a DAG; cannot topologically sort." << endl;
                }
                else {
                     cout << "Topological order (valid sequence):" << endl;
                     for (auto& id : order) {
                        cout << id << endl;
                     }
                }
                break;
            }

            case 5: {
                // Inline prerequisite graph
                if (allCourses.empty()) {
                    cout << "Please load the course data first (option 1)." << endl;
                    break;
                }
                printInlineGraph(allCourses);
                break;
            }
            
            // *New* 
            // Case for loading all courses in the database
            case 6: {
                repo.open("courses.db");
                auto rows = repo.getAllCourses();
                allCourses = rows;
                // rebuild BST
                coursesBST = CourseBST();          // or call a clear() if you add one
                for (const auto& c : rows) coursesBST.Insert(c);
                std::cout << "Loaded " << rows.size() << " courses from DB.\n";
                break;
            }

            // *New* 
            // Case for inserting or updating new courses into the database
            case 7: {
                std::string id, name, prereqLine;
                std::cout << "Course ID: "; std::getline(std::cin >> std::ws, id);
                std::cout << "Course Name: "; std::getline(std::cin, name);
                std::cout << "Prereqs (comma-separated, optional): ";
                std::getline(std::cin, prereqLine);

                Course c; c.courseNum = normalizeId(id); c.courseName = name;
                if (!prereqLine.empty()) {
                    std::stringstream ss(prereqLine);
                    std::string tok;
                    while (std::getline(ss, tok, ',')) {
                        tok = normalizeId(trim(tok));
                        if (!tok.empty()) c.preReqs.push_back(tok);
                    }
                }
                repo.open("courses.db");
                repo.upsertCourse(c);
                std::cout << "Upserted.\n";
                break;
            }

            // *New* 
            // Case for deleting a course from the database
            case 8: {
                std::string id;
                std::cout << "Course ID to delete: ";
                std::getline(std::cin >> std::ws, id);
                repo.open("courses.db");
                if (repo.deleteCourse(normalizeId(id))) {
                    std::cout << "Deleted.\n";
                }
                else {
                    std::cout << "Not found.\n";
                }
                break;
            }

            case 9:
                cout << "Exiting program..." << endl;
                break;

            default:
                cout << "Invalid option. Please choose again." << endl;
                break;
        }

    }
    cout << "\nThank you for using the course planner!" << endl;
}