// main_101300683_101310636.cpp (Part A - without semaphores)

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <vector>
#include <signal.h>
#include "shared_memory.h"
#include "file_manager.h"
#include "ta_process.h"

using namespace std;

void print_usage(const char* program_name) {
    cout << "Usage: " << program_name << " <number_of_TAs>" << endl;
    cout << "  number_of_TAs: must be >= 2" << endl;
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    int num_tas = atoi(argv[1]);
    if (num_tas < 2) {
        cerr << "Error: Number of TAs must be at least 2" << endl;
        print_usage(argv[0]);
        return 1;
    }
    
    cout << "============================================================" << endl;
    cout << "    TA Exam Marking System (Part A - Unsynchronized)       " << endl;
    cout << "============================================================" << endl;
    cout << "Number of TAs: " << num_tas << endl;
    cout << "------------------------------------------------------------" << endl;
    
    // Initialize shared memory in parent process
    SharedMemory shared_mem;
    if (!shared_mem.initialize()) {
        cerr << "Error: Failed to initialize shared memory" << endl;
        return 1;
    }
    
    // Load initial rubric
    if (!shared_mem.load_rubric_from_file()) {
        cerr << "Error: Failed to load rubric" << endl;
        shared_mem.cleanup();
        return 1;
    }
    
    // Get list of exams to process
    vector<int> exam_list = FileManager::get_exam_list();
    if (exam_list.empty()) {
        cerr << "Error: No exam files found in " << FileManager::EXAM_DIR << endl;
        shared_mem.cleanup();
        return 1;
    }
    
    cout << "Found " << exam_list.size() << " exam files" << endl;
    
    // Load first exam
    if (!shared_mem.load_exam_from_file(exam_list[0], 0)) {
        cerr << "Error: Failed to load first exam" << endl;
        shared_mem.cleanup();
        return 1;
    }
    
    cout << "Starting marking process with student " << exam_list[0] << endl;
    cout << "============================================================" << endl << endl;
    
    // Create TA processes
    vector<pid_t> ta_pids;
    for (int i = 0; i < num_tas; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            cerr << "Error: Failed to create TA process " << i << endl;
            // Kill already created processes
            for (pid_t p : ta_pids) {
                kill(p, SIGTERM);
            }
            shared_mem.cleanup();
            return 1;
        }
        else if (pid == 0) {
            // Child process - TA
            cout << "[TA " << i << "] Process started (PID: " << getpid() << ")" << endl;
            
            // Create TA process object and run
            TAProcess ta(i, &shared_mem, exam_list, nullptr);
            ta.run();
            
            // TA finished
            cout << "[TA " << i << "] Process terminating" << endl;
            exit(0);
        }
        else {
            // Parent process
            ta_pids.push_back(pid);
        }
    }
    
    // Parent waits for all TAs to complete
    cout << "[MAIN] All TA processes created, waiting for completion..." << endl << endl;
    
    for (size_t i = 0; i < ta_pids.size(); i++) {
        int status;
        pid_t pid = waitpid(ta_pids[i], &status, 0);
        if (pid > 0) {
            cout << "[MAIN] TA process " << pid << " (TA " << i << ") terminated with status " 
                 << WEXITSTATUS(status) << endl;
        }
    }
    
    cout << endl << "============================================================" << endl;
    cout << "         All TAs have finished marking exams                " << endl;
    cout << "============================================================" << endl;
    
    // Display final rubric
    cout << "\nFinal Rubric State:" << endl;
    RubricData* rubric = shared_mem.get_rubric_data();
    for (int i = 0; i < 5; i++) {
        cout << "  " << rubric->rubric_text[i] << endl;
    }
    
    // Cleanup shared memory
    shared_mem.cleanup();
    
    cout << "\nProgram completed successfully" << endl;
    return 0;
}