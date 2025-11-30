#include "file_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <cstring>

const std::string FileManager::RUBRIC_FILENAME = "data/rubric.txt";
const std::string FileManager::EXAM_DIR = "data/exams/";

std::vector<int> FileManager::get_exam_list() {
    std::vector<int> exam_list;
    DIR* dir = opendir(EXAM_DIR.c_str());
    
    if (!dir) {
        std::cerr << "Error: Could not open exam directory: " << EXAM_DIR << std::endl;
        return exam_list;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Look for files matching "exam_XXXX.txt"
        if (filename.find("exam_") == 0 && filename.find(".txt") != std::string::npos) {
            // Extract student number
            std::string num_str = filename.substr(5, 4); // Get XXXX from exam_XXXX.txt
            int student_num = std::stoi(num_str);
            exam_list.push_back(student_num);
        }
    }
    closedir(dir);
    
    // Sort in ascending order
    std::sort(exam_list.begin(), exam_list.end());
    
    return exam_list;
}

bool FileManager::read_exam_file(int student_number, int& student_num_out) {
    std::string filename = get_exam_filename(student_number);
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open exam file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    // Read first line which should contain "Student: XXXX"
    if (std::getline(file, line)) {
        // Extract number after "Student: "
        size_t pos = line.find("Student:");
        if (pos != std::string::npos) {
            std::string num_str = line.substr(pos + 8); // Skip "Student:"
            // Remove whitespace
            num_str.erase(0, num_str.find_first_not_of(" \t\r\n"));
            student_num_out = std::stoi(num_str);
            file.close();
            return true;
        }
    }
    
    file.close();
    return false;
}

bool FileManager::read_rubric_file(char rubric[][100]) {
    std::ifstream file(RUBRIC_FILENAME);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open rubric file: " << RUBRIC_FILENAME << std::endl;
        return false;
    }
    
    std::string line;
    int line_num = 0;
    
    while (std::getline(file, line) && line_num < 5) {
        strncpy(rubric[line_num], line.c_str(), 99);
        rubric[line_num][99] = '\0'; // Ensure null termination
        line_num++;
    }
    
    file.close();
    return (line_num == 5); // Should have exactly 5 lines
}

bool FileManager::write_rubric_file(const char rubric[][100]) {
    std::ofstream file(RUBRIC_FILENAME);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to rubric file: " << RUBRIC_FILENAME << std::endl;
        return false;
    }
    
    for (int i = 0; i < 5; i++) {
        file << rubric[i] << std::endl;
    }
    
    file.close();
    return true;
}

std::string FileManager::get_exam_filename(int student_number) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "exam_%04d.txt", student_number);
    return EXAM_DIR + std::string(buffer);
}