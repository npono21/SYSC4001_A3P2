#ifndef CONCURRENT_PROCESSES_H
#define CONCURRENT_PROCESSES_H

#include <stddef.h>

#define SHARED_RUBRIC "rubric_shm_obj" // name of rubric shared memory object
#define MAX_RUBRIC_ENTRIES 50          // generic cap on entries up to 50, can be changed

#define SHARED_EXAM "exam_shm_object" // name of exam shared memory object, will have # appended to end to signify which exam specifically
#define MAX_EXAM_ENTRIES 50           // generic cap on entries up to 50, can be changed

#define SEMAOPHORE_KEY 20254001 // key for semaphore for Part 2b

// Struct which will contain all contents of the rubric in shared memory
typedef struct
{
    int exercise_number[MAX_RUBRIC_ENTRIES];
    char exam_text[MAX_RUBRIC_ENTRIES];
    int entries_loaded;
} rubric_shared_data;

// Struct which will contain all contents of an exam file in shared memory
typedef struct
{
    int student_number;
    char q1_status;
    char q2_status;
    char q3_status;
    char q4_status;
    char q5_status;
    int entries_loaded;
} exam_file_shared_data;

/**
 * @brief Create a hared Memory Rubric object with size sizeof(rubric_shared_data) struct
 *
 * @return *rubric_shared_data A pointer to the rubric in shared memory
 */
rubric_shared_data *createSharedMemRubric();

/**
 * @brief Read the contents of rubric.txt into the shared memory rubric
 *
 * @param *rubric A pointer to the rubric in shared memory shared we want to load with data
 * @return *rubric_shared_data A pointer to the loaded rubric in shared memory
 */
rubric_shared_data *load_rubric(rubric_shared_data *rubric);

/**
 * @brief Create a Shared Memory Exam object with size sizeof(exam_file_shared_data) struct
 *
 * @param exam_shm_name The name of the shared memory exam we want to create
 * @return *xam_file_shared_data A pointer to the shared memory exam object
 */
exam_file_shared_data *createSharedMemExam(char *exam_shm_name);

/**
 * @brief Read the contents of exam1.txt into the shared memory exam object
 *        (struct holds a single student_number as char, status of each individual question (0 = unmarked, 1 = marked,) and entries_loaded)
 *
 * @param exam A pointer to shared memory exam object we want to load with data
 * @param exam_file_name The name of the exam file we want to load into shared memory from exams/ dir
 * @param ta_num The number of the TA responsible for loading the exam
 * @return *exam_file_shared_data A pointer to the loaded exam in shared memory
 */
exam_file_shared_data *load_exam(exam_file_shared_data *exam, const char *exam_file_name, int ta_num);

/**
 * @brief Access the rubric in shared memory
 *
 * @return *rubric_shared_data A pointer to the rubric in shared memory
 */
rubric_shared_data *accessSharedMemRubric();

/**
 * @brief Access the exam stored in shared memory
 *
 * @return *exam_file_shared_data A pointer to the exam in shared memory
 */
exam_file_shared_data *accessSharedMemExam(char *exam_shm_name);

/**
 * @brief Determine if the exam is fully marked
 * It is fully marked is all question status's i.e., q1_status, q2_status, etc, have a value of 1
 *
 * @param exam The exam in shared memory we want to check
 * @param exam_shm_name The name of the exam in shared memory (purely for printing purposes)
 * @return int Status if fully marked or not
 */
int exam_fully_marked(exam_file_shared_data *exam, char *exam_shm_name);

/**
 * @brief Helper function to generate a random delay value from 0.5 to 1.0 seconds
 *
 * @return double Randomly generated delay value from 0.5 to 1.0 seconds
 */
double random_delay_value();

/**
 * @brief Helper function to generate a random delay value from 1.0 to 2.0 seconds
 * To simulate the amount of time it takes to correct a question
 *
 * @return double Randomly generated delay value from 1.0 to 2.0 seconds
 */
double random_correcting_delay();

/**
 * @brief Function to actually write the rubric corrections to the hardcopy rubric.txt file in the directory
 *
 * @param rubric Pointer to the rubric in shared memory
 */
void correct_hardcopy_rubric(rubric_shared_data *rubric);

/**
 * @brief Function to check if a rubric line needs to be correct according to random generated num (1 or 0)
 * If the rubric line needs to be correct, increment and ASCII character by 1
 * If the line does not need to be corrected, do nothing to it
 *
 * @param rubric Pointer to the rubric in shared memory
 * @param ta Number of the TA doing the correction for printing purposes
 */
void check_and_correct_rubric(rubric_shared_data *rubric, int ta);

/**
 * @brief Check if the exam question is already marked through q<x>_status
 * If question status is 1, it is already marked
 * If question status is 0, it is not yet marked
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_q_to_mark Specific question to check if it is marked
 * @return int If 1 is returned, question is marked, if 0, not marked
 */
int is_exam_q_marked(exam_file_shared_data *exam, int exam_q_to_mark);

/**
 * @brief Now that the question status is checked if it is marked, we can mark it
 * i.e., change its status from 0 to 1
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_q_to_mark Specific question to mark
 */
void mark_question(exam_file_shared_data *exam, int exam_q_to_mark);

/**
 * @brief Write the shared memory exam to the actual "harcopy" .txt exam file
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_file_name Name of the exam file to write to, i.e., "exam1", "exam2", etc.
 * @param question_to_correct The specific exam question that needs to be modified in the harcopy exam file since it's been corrected
 */
void correct_hardcopy_exam(exam_file_shared_data *exam, char *exam_file_name, int question_to_correct);

/**
 * @brief Get an array of exam file names that are contained in the exams/ directory
 * ex. [exam1, exam2, exam3, etc.]...
 *
 * @param exam_count The number of files within the exams/ directory
 * @return char** The array of exam file names
 */
char **list_exams(int *exam_count);

/**
 * @brief Helper function to remove specific element from an array
 * This will be used to remove an already marked question from the array of all question number
 *
 * @param arr Array to remove an element from
 * @param size Size of the array
 * @param index Index of the element to remove
 * @return int Returns the size of the new array
 */
int remove_by_index(int arr[], int size, int index);

/**
 * @brief Create a TA Process to run concurrently with other TA proccesses to mark questions on exams
 *
 * @param num_ta_processes Number of TA processes to create based on supplied command line args in main()
 * @param rubric Pointer to the rubric in shared memory to grade from
 * @param exam Pointer to the exam in shared memory to grade
 * @param exam_count Number of exams that need to be graded, depends on amount of exam files in exams/
 * @param exam_files Array of all the exam files in exams/
 * @param semaphore_id The ID (int) of the semaphore to be used for accessing critical sections of memory
 * @return pid_t* Return an array containing the pid's of all TA processes
 */
pid_t *create_ta_processes(int num_ta_processes,
                           rubric_shared_data *rubric,
                           exam_file_shared_data *exam,
                           int exam_count,
                           char **exam_files,
                           int semaphore_id);

/**
 * @brief Decrement semaphore counter so only one process can work in critical section
 *
 * @param semaphoreID The ID of the semaphore we want to use
 */
void waitSemaphore(int semaphoreID);

/**
 * @brief Increment semaphore counter so that another process can go work in the critical section
 *
 * @param semaphoreID The ID of the semaphore we want to use
 */
void signalSemaphore(int semaphoreID);

#endif