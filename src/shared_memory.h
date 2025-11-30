#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <string>

// Shared data structures
struct ExamData {
    int student_number;
    bool questions_marked[5];     // Track which questions are marked
    int questions_being_marked[5]; // TA ID marking each question (-1 if none)
    bool all_marked;              // Flag to indicate all questions are done
    int current_exam_index;       // Index in exam list
};

struct RubricData {
    char rubric_text[5][100];  // 5 questions, up to 100 chars each
};

class SharedMemory {
private:
    int shm_id_exam;
    int shm_id_rubric;
    ExamData* exam_data;
    RubricData* rubric_data;
    
public:
    SharedMemory();
    ~SharedMemory();
    
    bool initialize();
    bool cleanup();
    
    ExamData* get_exam_data();
    RubricData* get_rubric_data();
    
    bool load_exam_from_file(int student_number, int exam_index);
    bool load_rubric_from_file();
    bool save_rubric_to_file();
};

#endif 