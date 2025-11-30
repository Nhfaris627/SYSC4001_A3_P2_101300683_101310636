README for compiling and running this program

### Manual Compilation

**Part A:**
```bash
g++ -Wall -Wextra -std=c++11 \
    src/main_101300683_101310636.cpp \
    src/file_manager.cpp \
    src/shared_memory.cpp \
    src/ta_process.cpp \
    -o main_101300683_101310636
```

**Run with 3 TAs:**
```bash
./main_101300683_101310636 3
```

**Part B:**
```bash
g++ -Wall -Wextra -std=c++11 \
    src/main_sem_101300683_101310636.cpp \
    src/file_manager.cpp \
    src/shared_memory.cpp \
    src/ta_process.cpp \
    src/semaphore_manager.cpp \
    -o main_sem_101300683_101310636
```

**Run with 3 TAs:**
```bash
./main_sem_101300683_101310636 3
```

## Test Cases

### Test Case 1: Minimal TAs (2 TAs)
```bash
./main_101300683_101310636 2
./main_sem_101300683_101310636 2
```
**Purpose**: Verify basic functionality with minimum required TAs.

### Test Case 2: Standard Load (3 TAs)
```bash
./main_101300683_101310636 3
./main_sem_101300683_101310636 3
```
**Purpose**: Observe typical concurrent behavior and synchronization.

### Test Case 3: High Concurrency (5 TAs)
```bash
./main_101300683_101310636 5
./main_sem_101300683_101310636 5
```
**Purpose**: Stress test with higher contention for resources.

### Test Case 4: Race Condition Observation (Part A)
```bash
./main_101300683_101310636 4 | grep "Changed rubric"
```
**Purpose**: Highlight rubric modification conflicts in unsynchronized version.

### Test Case 5: Synchronization Verification (Part B)
```bash
./main_sem_101300683_101310636 4 | grep "Changed rubric"
```
**Purpose**: Verify sequential rubric modifications in synchronized version.
```
## Known Limitations

- Rubric modifications use ASCII increment (may produce unprintable characters after ~30 changes)
- No file locking for exam files (assumed single program instance)
- Terminal may not display extended ASCII characters properly
- To fix this you must manually edit the rubric.txt file after every run to have the original starting output of the following as described in the assignment:
  
1, A
2, B
3, C
4, D
5, E
