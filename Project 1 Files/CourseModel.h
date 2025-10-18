#pragma once
//=============================================================================
// CourseModel.h                                              *New Header File*
//-----------------------------------------------------------------------------
// global course defintions for both the main.cpp(CS 499 Enhanced Search Algorithm - Takoda Jennings.cpp) and Database.h

#include <string>
#include <vector>

// Single source of truth for the Course model used across the app
struct Course {
    std::string courseNum;
    std::string courseName;
    std::vector<std::string> preReqs;
};