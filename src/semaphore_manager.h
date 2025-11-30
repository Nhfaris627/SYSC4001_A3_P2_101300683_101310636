#ifndef SEMAPHORE_MANAGER_H
#define SEMAPHORE_MANAGER_H

#include <semaphore.h>
#include <string>

class SemaphoreManager {
private:
    // Semaphores for rubric access 
    sem_t* rubric_mutex;        // Protects rubric writes
    sem_t* reader_count_mutex;  // Protects reader_count variable
    int* reader_count;          // Number of processes reading rubric (in shared memory)
    int shm_id_reader_count;    // Shared memory ID for reader_count
    
    // Semaphore for exam loading
    sem_t* exam_load_mutex;     // Only one TA can load next exam
    
    // Semaphores for question marking (one per question)
    sem_t* question_mutexes[5]; // Prevent multiple TAs marking same question
    
public:
    SemaphoreManager();
    ~SemaphoreManager();
    
    // Initialize all semaphores
    bool initialize();
    
    // Clean up all semaphores
    bool cleanup();
    
    // Readers-Writers for rubric access
    void start_read_rubric();   // Call before reading rubric
    void end_read_rubric();     // Call after reading rubric
    void start_write_rubric();  // Call before writing rubric
    void end_write_rubric();    // Call after writing rubric
    
    // Question marking coordination
    bool try_mark_question(int question_num);     // Try to claim a question
    void finish_mark_question(int question_num);  // Release question after marking
    
    // Exam loading coordination
    void lock_exam_load();      // Call before loading next exam
    void unlock_exam_load();    // Call after loading next exam
};

#endif