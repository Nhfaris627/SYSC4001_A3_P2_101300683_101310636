#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

class FileManager {
public:
    static const std::string RUBRIC_FILENAME;
    static const std::string EXAM_DIR;
    
    // Get list of all exam student numbers in order
    static std::vector<int> get_exam_list();
    
    // Read student number from exam file
    static bool read_exam_file(int student_number, int& student_num_out);
    
    // Read rubric from file into array
    static bool read_rubric_file(char rubric[][100]);
    
    // Write rubric array back to file
    static bool write_rubric_file(const char rubric[][100]);
    
    // Get filename for a given student number
    static std::string get_exam_filename(int student_number);
};

#endif 