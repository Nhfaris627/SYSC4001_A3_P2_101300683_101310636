#!/bin/bash
# setup_repo.sh - Initial repository setup script
# Run this to create the basic directory structure and files

echo "=== SYSC4001 A3P2 Repository Setup ==="
echo ""

echo ""
echo "Setting up repository for students: $STUDENT1 and $STUDENT2"

# Create directory structure
echo "Creating directory structure..."
mkdir -p src
mkdir -p data/exams
mkdir -p docs

# Create rubric file
echo "Creating rubric file..."
cat > data/rubric.txt << EOF
1, A
2, B
3, C
4, D
5, E
EOF

# Create exam files
echo "Creating exam files..."
for i in $(seq -f "%04g" 1 20); do
    cat > "data/exams/exam_$i.txt" << EOF
Student: $i
Exam Questions:
1. [unmarked]
2. [unmarked]
3. [unmarked]
4. [unmarked]
5. [unmarked]
EOF
done

# Create termination exam
cat > "data/exams/exam_9999.txt" << EOF
Student: 9999
Exam Questions:
1. [unmarked]
2. [unmarked]
3. [unmarked]
4. [unmarked]
5. [unmarked]
EOF

echo "Created $(ls data/exams/ | wc -l) exam files"

# Create placeholder source files
echo "Creating source file templates..."

# Create empty cpp files with proper names
touch "src/main_${STUDENT1}_${STUDENT2}.cpp"
touch "src/main_sem_${STUDENT1}_${STUDENT2}.cpp"
touch src/ta_process.cpp
touch src/shared_memory.cpp
touch src/file_manager.cpp
touch src/semaphore_manager.cpp

# Create empty header files
touch src/ta_process.h
touch src/shared_memory.h
touch src/file_manager.h
touch src/semaphore_manager.h

# Update Makefile with student numbers
echo "Updating Makefile..."
sed -i "s/STUDENT1 = .*/STUDENT1 = $STUDENT1/" Makefile
sed -i "s/STUDENT2 = .*/STUDENT2 = $STUDENT2/" Makefile