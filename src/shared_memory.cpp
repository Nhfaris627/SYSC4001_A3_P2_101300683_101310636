#include "shared_memory.h"
#include "file_manager.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
#include <cstring>

SharedMemory::SharedMemory() : shm_id_exam(-1), shm_id_rubric(-1), 
                                 exam_data(nullptr), rubric_data(nullptr) {
}

SharedMemory::~SharedMemory() {
    // Detach if still attached
    if (exam_data != nullptr) {
        shmdt(exam_data);
    }
    if (rubric_data != nullptr) {
        shmdt(rubric_data);
    }
}

bool SharedMemory::initialize() {
    // Create shared memory for exam data
    key_t exam_key = ftok(".", 'E');
    if (exam_key == -1) {
        std::cerr << "Error: ftok failed for exam" << std::endl;
        return false;
    }
    
    shm_id_exam = shmget(exam_key, sizeof(ExamData), IPC_CREAT | 0666);
    if (shm_id_exam == -1) {
        std::cerr << "Error: shmget failed for exam" << std::endl;
        return false;
    }
    
    // Attach to exam shared memory
    exam_data = (ExamData*)shmat(shm_id_exam, nullptr, 0);
    if (exam_data == (void*)-1) {
        std::cerr << "Error: shmat failed for exam" << std::endl;
        return false;
    }
    
    // Initialize exam data
    exam_data->student_number = 0;
    exam_data->all_marked = false;
    exam_data->current_exam_index = 0;
    for (int i = 0; i < 5; i++) {
        exam_data->questions_marked[i] = false;
        exam_data->questions_being_marked[i] = -1;
    }
    
    // Create shared memory for rubric data
    key_t rubric_key = ftok(".", 'R');
    if (rubric_key == -1) {
        std::cerr << "Error: ftok failed for rubric" << std::endl;
        return false;
    }
    
    shm_id_rubric = shmget(rubric_key, sizeof(RubricData), IPC_CREAT | 0666);
    if (shm_id_rubric == -1) {
        std::cerr << "Error: shmget failed for rubric" << std::endl;
        return false;
    }
    
    // Attach to rubric shared memory
    rubric_data = (RubricData*)shmat(shm_id_rubric, nullptr, 0);
    if (rubric_data == (void*)-1) {
        std::cerr << "Error: shmat failed for rubric" << std::endl;
        return false;
    }
    
    // Initialize rubric data
    for (int i = 0; i < 5; i++) {
        memset(rubric_data->rubric_text[i], 0, 100);
    }
    
    std::cout << "[SHARED_MEM] Initialized successfully" << std::endl;
    return true;
}

bool SharedMemory::cleanup() {
    bool success = true;
    
    // Detach from shared memory
    if (exam_data != nullptr) {
        if (shmdt(exam_data) == -1) {
            std::cerr << "Error: shmdt failed for exam" << std::endl;
            success = false;
        }
        exam_data = nullptr;
    }
    
    if (rubric_data != nullptr) {
        if (shmdt(rubric_data) == -1) {
            std::cerr << "Error: shmdt failed for rubric" << std::endl;
            success = false;
        }
        rubric_data = nullptr;
    }
    
    // Remove shared memory segments
    if (shm_id_exam != -1) {
        if (shmctl(shm_id_exam, IPC_RMID, nullptr) == -1) {
            std::cerr << "Error: shmctl IPC_RMID failed for exam" << std::endl;
            success = false;
        }
    }
    
    if (shm_id_rubric != -1) {
        if (shmctl(shm_id_rubric, IPC_RMID, nullptr) == -1) {
            std::cerr << "Error: shmctl IPC_RMID failed for rubric" << std::endl;
            success = false;
        }
    }
    
    std::cout << "[SHARED_MEM] Cleaned up" << std::endl;
    return success;
}

ExamData* SharedMemory::get_exam_data() {
    return exam_data;
}

RubricData* SharedMemory::get_rubric_data() {
    return rubric_data;
}

bool SharedMemory::load_exam_from_file(int student_number, int exam_index) {
    int student_num;
    if (!FileManager::read_exam_file(student_number, student_num)) {
        return false;
    }
    
    exam_data->student_number = student_num;
    exam_data->all_marked = false;
    exam_data->current_exam_index = exam_index;
    
    // Reset all questions to unmarked
    for (int i = 0; i < 5; i++) {
        exam_data->questions_marked[i] = false;
        exam_data->questions_being_marked[i] = -1;
    }
    
    std::cout << "[SHARED_MEM] Loaded exam for student " << student_num << std::endl;
    return true;
}

bool SharedMemory::load_rubric_from_file() {
    if (!FileManager::read_rubric_file(rubric_data->rubric_text)) {
        return false;
    }
    
    std::cout << "[SHARED_MEM] Loaded rubric from file" << std::endl;
    return true;
}

bool SharedMemory::save_rubric_to_file() {
    if (!FileManager::write_rubric_file(rubric_data->rubric_text)) {
        return false;
    }
    
    return true;
}