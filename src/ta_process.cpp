#include "ta_process.h"
#include "file_manager.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

// Constructor for Part B with semaphores
TAProcess::TAProcess(int id, SharedMemory* shm, const std::vector<int>& exams, SemaphoreManager* sem) 
    : ta_id(id), shared_mem(shm), exam_list(exams) {
    // Seed random number generator with TA ID and time
    srand(time(nullptr) + ta_id);
}

double TAProcess::get_random_delay(double min, double max) {
    double random = (double)rand() / RAND_MAX;
    return min + random * (max - min);
}

void TAProcess::review_and_correct_rubric() {
    std::cout << "[TA " << ta_id << "] Reviewing rubric..." << std::endl;
    
    RubricData* rubric = shared_mem->get_rubric_data();
    
    // Review each of the 5 questions
    for (int q = 0; q < 5; q++) {
        double delay = get_random_delay(0.5, 1.0);
        usleep(delay * 1000000); // Convert to microseconds
        
        // Randomly decide if correction is needed (30% chance)
        bool needs_correction = (rand() % 100) < 30;
        
        if (needs_correction) {
            std::cout << "[TA " << ta_id << "] Detected error in rubric for question " 
                      << (q + 1) << ", correcting..." << std::endl;
            
            // Find the character after the comma
            std::string line = rubric->rubric_text[q];
            size_t comma_pos = line.find(',');
            
            if (comma_pos != std::string::npos && comma_pos + 2 < line.length()) {
                char current_char = line[comma_pos + 2];
                char next_char = current_char + 1;
                
                // Replace the character
                line[comma_pos + 2] = next_char;
                
                // Copy back to shared memory
                strncpy(rubric->rubric_text[q], line.c_str(), 99);
                rubric->rubric_text[q][99] = '\0';
                
                std::cout << "[TA " << ta_id << "] Changed rubric Q" << (q + 1) 
                          << " from '" << current_char << "' to '" << next_char << "'" << std::endl;
                
                // Save to file (Race Condition Expected)
                shared_mem->save_rubric_to_file();
                std::cout << "[TA " << ta_id << "] Saved rubric changes to file" << std::endl;
            }
        }
    }
    
    std::cout << "[TA " << ta_id << "] Finished reviewing rubric" << std::endl;
}

int TAProcess::select_question_to_mark() {
    ExamData* exam = shared_mem->get_exam_data();
    
    // Find an unmarked question
    for (int q = 0; q < 5; q++) {
        if (!exam->questions_marked[q] && exam->questions_being_marked[q] == -1) {
            // Mark as being marked by this TA (Race Condtion expected)
            exam->questions_being_marked[q] = ta_id;
            return q;
        }
    }
    
    return -1; // No questions available
}

void TAProcess::mark_question(int question_num, int student_number) {
    std::cout << "[TA " << ta_id << "] Marking question " << (question_num + 1) 
              << " for student " << student_number << std::endl;
    
    // Simulate marking time (1.0 to 2.0 seconds)
    double delay = get_random_delay(1.0, 2.0);
    usleep(delay * 1000000);
    
    // Mark as complete
    ExamData* exam = shared_mem->get_exam_data();
    exam->questions_marked[question_num] = true;
    exam->questions_being_marked[question_num] = -1;
    
    std::cout << "[TA " << ta_id << "] Finished marking question " << (question_num + 1) 
              << " for student " << student_number << std::endl;
    
    // Check if all questions are marked
    bool all_done = true;
    for (int i = 0; i < 5; i++) {
        if (!exam->questions_marked[i]) {
            all_done = false;
            break;
        }
    }
    
    if (all_done) {
        exam->all_marked = true;
        std::cout << "[TA " << ta_id << "] All questions marked for student " 
                  << student_number << std::endl;
    }
}

bool TAProcess::load_next_exam() {
    ExamData* exam = shared_mem->get_exam_data();
    int next_index = exam->current_exam_index + 1;
    
    if (next_index >= exam_list.size()) {
        std::cout << "[TA " << ta_id << "] No more exams to load" << std::endl;
        return false;
    }
    
    int next_student = exam_list[next_index];
    
    std::cout << "[TA " << ta_id << "] Loading next exam (student " << next_student << ")" << std::endl;
    
    // Race Condtion expected, multiple TAs might try to load same exam
    if (!shared_mem->load_exam_from_file(next_student, next_index)) {
        std::cerr << "[TA " << ta_id << "] Failed to load exam for student " << next_student << std::endl;
        return false;
    }
    
    std::cout << "[TA " << ta_id << "] Loaded exam for student " << next_student << std::endl;
    return true;
}

void TAProcess::run() {
    std::cout << "[TA " << ta_id << "] Starting work..." << std::endl;
    
    while (true) {
        ExamData* exam = shared_mem->get_exam_data();
        
        // Check if we've reached the termination exam
        if (exam->student_number == 9999) {
            std::cout << "[TA " << ta_id << "] Reached termination exam (9999), stopping" << std::endl;
            break;
        }
        
        // If current exam is complete, try to load next one
        if (exam->all_marked) {
            // Small delay to let others see it's complete
            usleep(100000); // 0.1 seconds
            
            if (!load_next_exam()) {
                break;
            }
            
            // Check again if it's the termination exam
            if (exam->student_number == 9999) {
                std::cout << "[TA " << ta_id << "] Loaded termination exam (9999), stopping" << std::endl;
                break;
            }
        }
        
        // Review and possibly correct rubric
        review_and_correct_rubric();
        
        // Try to mark a question
        int question = select_question_to_mark();
        
        if (question != -1) {
            mark_question(question, exam->student_number);
        } else {
            // No questions available, wait a bit
            std::cout << "[TA " << ta_id << "] No questions available, waiting..." << std::endl;
            usleep(200000); // 0.2 seconds
        }
    }
    
    std::cout << "[TA " << ta_id << "] Finished all work" << std::endl;
}