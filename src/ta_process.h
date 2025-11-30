#ifndef TA_PROCESS_H
#define TA_PROCESS_H

#include "shared_memory.h"
#include <string>
#include <vector>

class SemaphoreManager;

class TAProcess {
private:
    int ta_id;
    SharedMemory* shared_mem;
    std::vector<int> exam_list;
    SemaphoreManager* sem_manager;
    
    void review_and_correct_rubric();
    int select_question_to_mark();
    void mark_question(int question_num, int student_number);
    double get_random_delay(double min, double max);
    bool load_next_exam();
    
public:
    // Constructor for Part B with semaphores
    TAProcess(int id, SharedMemory* shm, const std::vector<int>& exams, SemaphoreManager* sem);
    void run();
};

#endif