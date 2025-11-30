// semaphore_manager.cpp
// Implementation of semaphore operations for synchronization

#include "semaphore_manager.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>

SemaphoreManager::SemaphoreManager() 
    : rubric_mutex(SEM_FAILED),
      reader_count_mutex(SEM_FAILED),
      reader_count(nullptr),
      shm_id_reader_count(-1),
      exam_load_mutex(SEM_FAILED)
{
    for (int i = 0; i < 5; i++) {
        question_mutexes[i] = SEM_FAILED;
    }
}

SemaphoreManager::~SemaphoreManager() {
    // Cleanup handled in cleanup() method
}

// Create all semaphores and shared memory for reader_count
bool SemaphoreManager::initialize() {
    std::cout << "[SEM] Initializing semaphores..." << std::endl;
    
    // Create shared memory for reader_count (needed for readers-writers)
    key_t key = ftok(".", 'C');
    if (key == -1) {
        std::cerr << "[SEM] Error: ftok failed for reader_count" << std::endl;
        return false;
    }
    
    shm_id_reader_count = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id_reader_count == -1) {
        std::cerr << "[SEM] Error: shmget failed for reader_count" << std::endl;
        return false;
    }
    
    reader_count = (int*)shmat(shm_id_reader_count, nullptr, 0);
    if (reader_count == (void*)-1) {
        std::cerr << "[SEM] Error: shmat failed for reader_count" << std::endl;
        return false;
    }
    *reader_count = 0;
    
    // Create rubric semaphores (readers-writers)
    rubric_mutex = sem_open("/rubric_mutex", O_CREAT, 0666, 1);
    if (rubric_mutex == SEM_FAILED) {
        std::cerr << "[SEM] Error: sem_open failed for rubric_mutex" << std::endl;
        return false;
    }
    
    reader_count_mutex = sem_open("/reader_count_mutex", O_CREAT, 0666, 1);
    if (reader_count_mutex == SEM_FAILED) {
        std::cerr << "[SEM] Error: sem_open failed for reader_count_mutex" << std::endl;
        return false;
    }
    
    // Create exam loading semaphore
    exam_load_mutex = sem_open("/exam_load_mutex", O_CREAT, 0666, 1);
    if (exam_load_mutex == SEM_FAILED) {
        std::cerr << "[SEM] Error: sem_open failed for exam_load_mutex" << std::endl;
        return false;
    }
    
    // Create question semaphores (one per question)
    for (int i = 0; i < 5; i++) {
        char sem_name[50];
        snprintf(sem_name, sizeof(sem_name), "/question_mutex_%d", i);
        question_mutexes[i] = sem_open(sem_name, O_CREAT, 0666, 1);
        if (question_mutexes[i] == SEM_FAILED) {
            std::cerr << "[SEM] Error: sem_open failed for " << sem_name << std::endl;
            return false;
        }
    }
    
    std::cout << "[SEM] All semaphores initialized successfully" << std::endl;
    return true;
}

// Close and remove all semaphores
bool SemaphoreManager::cleanup() {
    std::cout << "[SEM] Cleaning up semaphores..." << std::endl;
    
    if (rubric_mutex != SEM_FAILED) {
        sem_close(rubric_mutex);
        sem_unlink("/rubric_mutex");
    }
    
    if (reader_count_mutex != SEM_FAILED) {
        sem_close(reader_count_mutex);
        sem_unlink("/reader_count_mutex");
    }
    
    if (exam_load_mutex != SEM_FAILED) {
        sem_close(exam_load_mutex);
        sem_unlink("/exam_load_mutex");
    }
    
    for (int i = 0; i < 5; i++) {
        if (question_mutexes[i] != SEM_FAILED) {
            char sem_name[50];
            snprintf(sem_name, sizeof(sem_name), "/question_mutex_%d", i);
            sem_close(question_mutexes[i]);
            sem_unlink(sem_name);
        }
    }
    
    if (reader_count != nullptr) {
        shmdt(reader_count);
        reader_count = nullptr;
    }
    
    if (shm_id_reader_count != -1) {
        shmctl(shm_id_reader_count, IPC_RMID, nullptr);
    }
    
    std::cout << "[SEM] Semaphores cleaned up" << std::endl;
    return true;
}

// Readers-Writers: Acquire read access (multiple readers allowed)
void SemaphoreManager::start_read_rubric() {
    sem_wait(reader_count_mutex);
    (*reader_count)++;
    if (*reader_count == 1) {
        sem_wait(rubric_mutex);  // First reader locks out writers
    }
    sem_post(reader_count_mutex);
}

// Readers-Writers: Release read access
void SemaphoreManager::end_read_rubric() {
    sem_wait(reader_count_mutex);
    (*reader_count)--;
    if (*reader_count == 0) {
        sem_post(rubric_mutex);  // Last reader allows writers
    }
    sem_post(reader_count_mutex);
}

// Readers-Writers: Acquire write access (exclusive)
void SemaphoreManager::start_write_rubric() {
    sem_wait(rubric_mutex);
}

// Readers-Writers: Release write access
void SemaphoreManager::end_write_rubric() {
    sem_post(rubric_mutex);
}

// Try to claim a question for marking (non-blocking)
bool SemaphoreManager::try_mark_question(int question_num) {
    if (question_num < 0 || question_num >= 5) {
        return false;
    }
    return (sem_trywait(question_mutexes[question_num]) == 0);
}

// Release a question after marking
void SemaphoreManager::finish_mark_question(int question_num) {
    if (question_num >= 0 && question_num < 5) {
        sem_post(question_mutexes[question_num]);
    }
}

// Acquire exclusive access to load next exam
void SemaphoreManager::lock_exam_load() {
    sem_wait(exam_load_mutex);
}

// Release exam loading access
void SemaphoreManager::unlock_exam_load() {
    sem_post(exam_load_mutex);
}