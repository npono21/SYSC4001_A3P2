#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>

// purely for styling the printouts
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#include "concurent_processes_101182048_101324189.h"

/**
 * @brief Create a hared Memory Rubric object with size sizeof(rubric_shared_data) struct
 *
 * @return *rubric_shared_data A pointer to the rubric in shared memory
 */
rubric_shared_data *createSharedMemRubric()
{
    // remove name of the rubric if it already exists, no error occurs if not
    shm_unlink(SHARED_RUBRIC);

    // create the shared memory rubric
    int shm_fd = shm_open(SHARED_RUBRIC, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to create rubric!\n");
        return NULL;
    }

    // configure the size of the shared memory rubric
    if (ftruncate(shm_fd, sizeof(rubric_shared_data)) == -1)
    {
        fprintf(stderr, "Failed to configure the size of rubric!\n");
        close(shm_fd);
        return NULL;
    }

    // map the shared memory rubric into our memory space
    rubric_shared_data *rubric_ptr = mmap(0, sizeof(rubric_shared_data),
                                          PROT_READ | PROT_WRITE, MAP_SHARED,
                                          shm_fd, 0);
    if (rubric_ptr == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory rubric!\n");
        close(shm_fd);
        return NULL;
    }

    close(shm_fd);

    printf(ANSI_COLOR_RED "\n------------CREATING SHARED MEMORY OBJECT FOR RUBRIC------------" ANSI_COLOR_RESET "\n");
    printf("Shared memory object for the rubric has been created!\n");

    return rubric_ptr;
}

/**
 * @brief Read the contents of rubric.txt into the shared memory rubric
 *
 * @param *rubric A pointer to the rubric in shared memory shared we want to load with data
 * @return *rubric_shared_data A pointer to the loaded rubric in shared memory
 */
rubric_shared_data *load_rubric(rubric_shared_data *rubric)
{
    if (!rubric)
        return NULL;

    rubric->entries_loaded = 0;

    FILE *fp = fopen("rubric/rubric.txt", "r");

    if (!fp)
    {
        perror("Could not open rubric.txt file!");
        return NULL;
    }

    int exercise_number;
    char exam_text;
    int i = 0;

    while (i < MAX_RUBRIC_ENTRIES &&
           fscanf(fp, " %d , %c", &exercise_number, &exam_text) == 2)
    {
        rubric->exercise_number[i] = exercise_number;
        rubric->exam_text[i] = exam_text;
        i++;
    }

    rubric->entries_loaded = i;
    fclose(fp);

    printf(ANSI_COLOR_RED "\n------------LOADING RUBRIC.TXT INTO SHARED MEMORY------------" ANSI_COLOR_RESET "\n");
    printf("Rubric file rubric.txt successfully loaded into shared memory!\n");

    return rubric;
}

/**
 * @brief Create a Shared Memory Exam object with size sizeof(exam_file_shared_data) struct
 *
 * @param exam_shm_name The name of the shared memory exam we want to create
 * @return *xam_file_shared_data A pointer to the shared memory exam object
 */
exam_file_shared_data *createSharedMemExam(char *exam_shm_name)
{
    // remove name of the exam if it already exists, no error occurs if not
    shm_unlink(exam_shm_name);

    // create the shared memory exam
    int shm_fd = shm_open(exam_shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to create exam!\n");
        return NULL;
    }

    // configure the size of the shared memory exam
    if (ftruncate(shm_fd, sizeof(exam_file_shared_data)) == -1)
    {
        fprintf(stderr, "Failed to configure the size of exam!\n");
        close(shm_fd);
        return NULL;
    }

    // map the shared memory exam into our memory space
    exam_file_shared_data *exam_ptr = mmap(0, sizeof(exam_file_shared_data),
                                           PROT_READ | PROT_WRITE, MAP_SHARED,
                                           shm_fd, 0);
    if (exam_ptr == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory exam!\n");
        close(shm_fd);
        return NULL;
    }

    close(shm_fd);

    printf(ANSI_COLOR_RED "\n------------CREATING SHARED MEMORY OBJECT FOR EXAMS------------" ANSI_COLOR_RESET "\n");
    printf("Shared memory object for the exams has been created!\n");
    return exam_ptr;
}

/**
 * @brief Read the contents of exam1.txt into the shared memory exam object
 *        (struct holds a single student_number as char, status of each individual question (0 = unmarked, 1 = marked,) and entries_loaded)
 *
 * @param exam A pointer to shared memory exam object we want to load with data
 * @param exam_file_name The name of the exam file we want to load into shared memory from exams/ dir
 * @param ta_num The number of the TA responsible for loading the exam
 * @return *exam_file_shared_data A pointer to the loaded exam in shared memory
 */
exam_file_shared_data *load_exam(exam_file_shared_data *exam, const char *exam_file_name, int ta_num)
{
    if (!exam)
        return NULL;

    exam->entries_loaded = 0;

    // create file path for chosen exam file, i.e., exam4 --> exams/exam4.txt
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "exams/%s.txt", exam_file_name);

    // open the chosen exam file
    FILE *fp = fopen(file_path, "r");
    if (!fp)
    {
        printf("Could not open %s.txt file!\n", exam_file_name);
        return NULL;
    }

    int sn, s1, s2, s3, s4, s5;

    // read ALL 6 values (student number + status for 5 questions)
    if (fscanf(fp, "%d %d %d %d %d %d",
               &sn, &s1, &s2, &s3, &s4, &s5) == 6)
    {
        exam->student_number = sn;
        exam->q1_status = s1;
        exam->q2_status = s2;
        exam->q3_status = s3;
        exam->q4_status = s4;
        exam->q5_status = s5;

        exam->entries_loaded = 1; // we loaded one exam record
    }
    else
    {
        exam->entries_loaded = 0;
    }

    fclose(fp);
    printf(ANSI_COLOR_RED "\n------------LOADING EXAM FILE INTO SHARED MEMORY------------" ANSI_COLOR_RESET "\n");
    printf("TA %d loaded exam file: %s successfully into shared memory!\n", ta_num, exam_file_name);
    return exam;
}

/**
 * @brief Access the rubric in shared memory
 *
 * @return *rubric_shared_data A pointer to the rubric in shared memory
 */
rubric_shared_data *accessSharedMemRubric()
{
    // open the shared memory rubric created in main
    int shm_fd = shm_open(SHARED_RUBRIC, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to open shared memory rubric %s...\n", SHARED_RUBRIC);
        return (rubric_shared_data *)-1; // return error code
    }

    // map the shared memory rubric into our memory space
    rubric_shared_data *rubric = (rubric_shared_data *)mmap(0, sizeof(rubric_shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (rubric == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory object %s...\n", SHARED_RUBRIC);
        close(shm_fd);
        return (rubric_shared_data *)-1; // return error code
    }

    close(shm_fd);
    // return pointer to rubric
    return rubric;
}

/**
 * @brief Access the exam stored in shared memory
 *
 * @return *exam_file_shared_data A pointer to the exam in shared memory
 */
exam_file_shared_data *accessSharedMemExam(char *exam_shm_name)
{
    // open the shared memory exam created in main
    int shm_fd = shm_open(exam_shm_name, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to open shared memory exam %s...\n", exam_shm_name);
        return (exam_file_shared_data *)-1; // return error code
    }

    // map the shared memory exam into our memory space
    exam_file_shared_data *exam = (exam_file_shared_data *)mmap(0, sizeof(exam_file_shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (exam == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory exam %s...\n", exam_shm_name);
        close(shm_fd);
        return (exam_file_shared_data *)-1; // return error code
    }

    close(shm_fd);
    // return pointer to exam
    printf(ANSI_COLOR_RED "\n------------ACCESSING EXAM FROM SHARED MEMORY------------" ANSI_COLOR_RESET "\n");
    printf("%s successfully accessed from shared memory!\n", exam_shm_name);
    return exam;
}

/**
 * @brief Determine if the exam is fully marked
 * It is fully marked is all question status's i.e., q1_status, q2_status, etc, have a value of 1
 *
 * @param exam The exam in shared memory we want to check
 * @param exam_shm_name The name of the exam in shared memory (purely for printing purposes)
 * @return int Status if fully marked or not
 */
int exam_fully_marked(exam_file_shared_data *exam, char *exam_shm_name)
{
    if (exam->q1_status == 1 &&
        exam->q2_status == 1 &&
        exam->q3_status == 1 &&
        exam->q4_status == 1 &&
        exam->q5_status == 1)
    {
        printf("Exam %s for student %04d is fully marked!\n", exam_shm_name, exam->student_number);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Helper function to generate a random delay value from 0.5 to 1.0 seconds
 *
 * @return double Randomly generated delay value from 0.5 to 1.0 seconds
 */
double random_delay_value()
{
    srand(time(NULL)); // seed rand()
    double delay_value = 0.5 + ((double)rand() / (double)RAND_MAX) * 0.5;
    return round(delay_value * 10.0) / 10.0;
}

/**
 * @brief Helper function to generate a random delay value from 1.0 to 2.0 seconds
 * To simulate the amount of time it takes to correct a question
 *
 * @return double Randomly generated delay value from 1.0 to 2.0 seconds
 */
double random_correcting_delay()
{
    srand(time(NULL));       // seed rand()
    int r = rand() % 11;     // generates 0–10
    return 1.0 + (r / 10.0); // converts to 1.0–2.0 in 0.1 increments
}

/**
 * @brief Function to actually write the rubric corrections to the hardcopy rubric.txt file in the directory
 *
 * @param rubric Pointer to the rubric in shared memory
 */
void correct_hardcopy_rubric(rubric_shared_data *rubric)
{

    FILE *fp = fopen("rubric/rubric.txt", "w");

    if (!fp)
    {
        perror("Could not open rubric.txt file!");
        exit(1);
    }

    for (int i = 0; i < rubric->entries_loaded; i++)
    {
        fprintf(fp, "%d,%c\n", rubric->exercise_number[i], rubric->exam_text[i]);
    }

    fclose(fp);
}

/**
 * @brief Function to check if a rubric line needs to be correct according to random generated num (1 or 0)
 * If the rubric line needs to be correct, increment and ASCII character by 1
 * If the line does not need to be corrected, do nothing to it
 *
 * @param rubric Pointer to the rubric in shared memory
 * @param ta Number of the TA doing the correction for printing purposes
 */
void check_and_correct_rubric(rubric_shared_data *rubric, int ta)
{
    sleep(1); // sleep a little bit to prevent the printout being laggy
    printf(ANSI_COLOR_RED "\n------------CORRECTING RUBRIC------------" ANSI_COLOR_RESET "\n");
    for (int i = 0; i < rubric->entries_loaded; i++)
    {
        double delay_val = random_delay_value();
        // convert deley value in seconds to microseconds for the usleep() function to delay execution
        int micro = (int)(delay_val * 1000000);
        usleep(micro);

        // if random value is 1, line in rubric must be corrected
        if ((rand() % 2) == 1)
        {
            rubric->exam_text[i] = rubric->exam_text[i] + 1;
            printf("TA #%d found rubric value %c incorrect. Correcting to %c!\n", ta, rubric->exam_text[i], rubric->exam_text[i] + 1);
            // if the original value is the maximum ASCII value, we re-start at the first visible printable character which is "!", or 33
            if (rubric->exam_text[i] == 126)
            {
                printf("TA #%d Reached maximum ASCII Value, resetting to %c whose ASCII value is %d\n", ta, 33, 33);
                rubric->exam_text[i] = 32;
            }
        }
        else
        {
            printf("TA #%d found rubric value %c correct!\n", ta, rubric->exam_text[i]);
        }
    }

    // writing correct rubric from shared memory back into hardcopy rubric.txt file
    correct_hardcopy_rubric(rubric);
}

/**
 * @brief Check if the exam question is already marked through q<x>_status
 * If question status is 1, it is already marked
 * If question status is 0, it is not yet marked
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_q_to_mark Specific question to check if it is marked
 * @return int If 1 is returned, question is marked, if 0, not marked
 */
int is_exam_q_marked(exam_file_shared_data *exam, int exam_q_to_mark)
{
    if (exam_q_to_mark == 0 && exam->q1_status == 1)
    {
        return 1;
    }
    else if (exam_q_to_mark == 1 && exam->q2_status == 1)
    {
        return 1;
    }
    else if (exam_q_to_mark == 2 && exam->q3_status == 1)
    {
        return 1;
    }
    else if (exam_q_to_mark == 3 && exam->q4_status == 1)
    {
        return 1;
    }
    else if (exam_q_to_mark == 4 && exam->q5_status == 1)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Now that the question status is checked if it is marked, we can mark it
 * i.e., change its status from 0 to 1
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_q_to_mark Specific question to mark
 */
void mark_question(exam_file_shared_data *exam, int exam_q_to_mark)
{
    printf(ANSI_COLOR_RED "\n------------MARKING EXAM QUESTION------------" ANSI_COLOR_RESET "\n");
    // as per assignment specifications, if a file with a student number of 9999 is reached
    // all execution finishes
    if (exam->student_number == 9999)
    {
        printf(ANSI_COLOR_RED "\n------------STUDENT NUMBER 9999 DETECTED. EXITING PROGRAM NOW.------------" ANSI_COLOR_RESET "\n");
        kill(0, SIGTERM);
    }

    double correcting_delay = random_correcting_delay();
    // convert deley value in seconds to microseconds for the usleep() function to delay execution
    int micro = (int)(correcting_delay * 1000000);
    usleep(micro);

    if (exam_q_to_mark == 0)
    {
        exam->q1_status = 1;
    }
    else if (exam_q_to_mark == 1)
    {
        exam->q2_status = 1;
    }
    else if (exam_q_to_mark == 2)
    {
        exam->q3_status = 1;
    }
    else if (exam_q_to_mark == 3)
    {
        exam->q4_status = 1;
    }
    else if (exam_q_to_mark == 4)
    {
        exam->q5_status = 1;
    }
}

/**
 * @brief Write the shared memory exam to the actual "harcopy" .txt exam file
 *
 * @param exam Pointer to the exam in shared memory
 * @param exam_file_name Name of the exam file to write to, i.e., "exam1", "exam2", etc.
 * @param question_to_correct The specific exam question that needs to be modified in the harcopy exam file since it's been corrected
 */
void correct_hardcopy_exam(exam_file_shared_data *exam, char *exam_file_name, int question_to_correct)
{
    // create the file path with the supplied exam_file_name argument, open the file for reading and writing
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "exams/%s.txt", exam_file_name);

    FILE *fp = fopen(file_path, "r+");
    if (!fp)
    {
        printf("Could not open %s.txt file!", exam_file_name);
        exit(1);
    }

    // store all six lines from opened exam .txt file for later use
    char lines[6][64];

    for (int i = 0; i < 6; i++)
    {
        if (fgets(lines[i], sizeof(lines[i]), fp) == NULL)
        {
            fclose(fp);
            return;
        }
    }

    int new_value;

    // select the question we corrected and get's its current status
    if (question_to_correct == 0)
        new_value = exam->q1_status;
    else if (question_to_correct == 1)
        new_value = exam->q2_status;
    else if (question_to_correct == 2)
        new_value = exam->q3_status;
    else if (question_to_correct == 3)
        new_value = exam->q4_status;
    else if (question_to_correct == 4)
        new_value = exam->q5_status;
    else
    {
        fclose(fp);
        return;
    }

    // update the previously stored six lines with the new line based off the previous if...else if...else block
    snprintf(lines[question_to_correct + 1], sizeof(lines[0]), "%d\n", new_value);

    fseek(fp, 0, SEEK_SET); // rewind the file to the start to begin printing new lines down below

    // rewrite the specific exam .txt file from the stored content in lines[][], which is updated with the new content
    for (int i = 0; i < 6; i++)
        fputs(lines[i], fp);

    // flush and truncate the file to only what we need based of final file position
    long pos = ftell(fp);
    if (pos != -1)
    {
        fflush(fp);
        if (ftruncate(fileno(fp), pos) == -1)
            perror("ftruncate failed");
    }

    fclose(fp);
}

/**
 * @brief Get an array of exam file names that are contained in the exams/ directory
 * ex. [exam1, exam2, exam3, etc.]...
 *
 * The idea is that if we want to increase or decrease number of exam files in exams/ dir
 * We can do so without worrying about breaking the rest of the code
 *
 * @param exam_count The number of files within the exams/ directory
 * @return char** The array of exam file names
 */
char **list_exams(int *exam_count)
{
    // open directoru stream from exams/ directory
    struct dirent *de;
    DIR *dr = opendir("exams/");

    int exam_file_count = 0;

    if (dr == NULL)
    {
        printf("Could not open current directory");
        exit(1);
    }

    // count how many files are in the directory while ignoring '.' and '..'
    // I don't even know what they are but when I printed the directory array in a previous iteraction, those were in there,
    // so now we were getting rid of them, so the directory array only contains exam files
    while ((de = readdir(dr)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        exam_file_count++;
    }

    // reset directory stream to pass loop a second time to write filenames to array after counting how many files we have
    rewinddir(dr);

    // allocate memory for the array based off the total file count in the directory
    char **exam_files = malloc(sizeof(char *) * exam_file_count);
    if (exam_files == NULL)
    {
        closedir(dr);
        return NULL;
    }

    int i = 0;

    // read every single final name in the exams/ directory
    // remove the extension, i.e., .txt
    // allocate memory into the array for the filename
    // store the filename into the array, i.e., "exam1", "exam2", etc.
    while ((de = readdir(dr)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        char *exam_name = de->d_name;
        size_t len = strlen(exam_name);

        // remove the .txt extension
        if (len > 4 && strcmp(exam_name + len - 4, ".txt") == 0)
        {
            len -= 4;
        }

        // allocate memory to store filename in array
        exam_files[i] = malloc(len + 1);
        if (exam_files[i] == NULL)
        {
            return NULL;
        }

        // store (copy) filename with .txt into array
        strncpy(exam_files[i], exam_name, len);
        exam_files[i][len] = '\0';
        i++;
    }

    closedir(dr);                  // close the directory stream
    *exam_count = exam_file_count; // store the number of total exam files
    return exam_files;
}

/**
 * @brief Helper function to remove specific element from an array
 * This will be used to remove an already marked question from the array of all question number
 *
 * @param arr Array to remove an element from
 * @param size Size of the array
 * @param index Index of the element to remove
 * @return int Returns the size of the new array
 */
int remove_by_index(int arr[], int size, int index)
{
    if (index < 0 || index >= size)
        return size;

    for (int i = index; i < size - 1; i++)
        arr[i] = arr[i + 1];

    return size - 1;
}

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
                           int semaphore_id)
{

    printf(ANSI_COLOR_RED "\n------------CREATING TA'S------------" ANSI_COLOR_RESET "\n");

    // allocate memory to array of TA process ID's so we can wait for them in main
    pid_t *ta_pids = malloc(sizeof(pid_t) * num_ta_processes);
    if (ta_pids == NULL)
    {
        printf("Could not allocate memory for array to store TA PID's!\n");
        exit(1);
    }

    // loop for the amount of ta processes the user specified from the command line arg
    for (int i = 0; i < num_ta_processes; i++)
    {
        printf("Creating TA #%d now!\n", i + 1);
        pid_t pid = fork(); // create the TA process as a CHILD process

        // error in creating the TA process, exit with error
        if (pid == -1)
        {
            printf("TA Process #%d could not be created!\n", i + 1);
            free(ta_pids); // we can free the array of TA process PID's since we got an error when creating the TA process
            exit(1);
        }
        // success in creating the TA process
        else if (pid == 0)
        {
            sleep(1); // sleep a little bit to prevent the printout being laggy
            printf(" --- TA Process #%d created - PID is %d --- \n", i + 1, getpid());
            // ------ access the rubric stored in shared memory and correct it according to assignment specification ------
            rubric = accessSharedMemRubric();
            if (rubric == (rubric_shared_data *)-1)
                exit(1);

            waitSemaphore(semaphore_id); // lock semaphore for the specific process
            printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) LOCKING SEMAPHORE (%d) FOR RUBRIC CORRECTING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);
            check_and_correct_rubric(rubric, i + 1);
            printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) UNLOCKING SEMAPHORE (%d) FROM RUBRIC CORRECTING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);
            signalSemaphore(semaphore_id); // unlock semaphore from the specific process

            srand(getpid()); // seed the rand() function for future use in the TA process

            // run this loop for however many exams need to be marked, which is passed to create_ta_processes() as a param from main()
            for (int j = exam_count - 1; j >= 0; j--)
            {
                // extract the first exam name from the exam_files[] array which holds all exam file names
                // gotten directly from the exams/ directory
                char *exam_file_name = exam_files[j];

                waitSemaphore(semaphore_id); // lock semaphore for the specific process
                printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) LOCKING SEMAPHORE (%d) FOR EXAM LOADING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);
                // load the first exam's data into shared memory, exit with error if unable to
                if (!load_exam(exam, exam_file_name, i + 1))
                {
                    fprintf(stderr, "Failed to load exam %s!\n", exam_file_name);
                    exit(1);
                }
                printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) UNLOCKING SEMAPHORE (%d) FROM EXAM MARKING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);
                signalSemaphore(semaphore_id); // unlock semaphore from the specific process

                // run this loop until every single question is marked in the exam
                while (exam_fully_marked(exam, exam_file_name) != 1)
                {
                    // array of question numbers, used later on to ensure the same question isn't "randomly" selected to be marked again
                    int question_number_arr[] = {0, 1, 2, 3, 4};
                    int length_question_number_arr =
                        sizeof(question_number_arr) / sizeof(question_number_arr[0]);

                    int exam_q_to_mark = rand() % length_question_number_arr; // randomly choose the index of the question_number_arr[], i.e., the next question to mark

                    // first ensure the question is not already marked, then mark it
                    if (!is_exam_q_marked(exam, question_number_arr[exam_q_to_mark]))
                    {
                        waitSemaphore(semaphore_id); // lock semaphore for the specific process
                        printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) LOCKING SEMAPHORE (%d) FOR EXAM MARKING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);

                        printf(ANSI_COLOR_RED "\n------------CORRECTING EXAM QUESTION------------" ANSI_COLOR_RESET "\n");

                        // program may exit from mark_question if student number on exam is 9999
                        mark_question(exam, question_number_arr[exam_q_to_mark]);

                        // write the updated question status as marked to the actual exam .txt file in exams/
                        correct_hardcopy_exam(exam, exam_file_name, question_number_arr[exam_q_to_mark]);

                        printf("TA #%d marked question %d on exam %s for student %04d\n",
                               i + 1,
                               question_number_arr[exam_q_to_mark] + 1,
                               exam_file_name,
                               exam->student_number);

                        printf(ANSI_COLOR_RED "\n------------MARKING EXAM QUESTION CORRECT ON EXAM FILE------------" ANSI_COLOR_RESET "\n");

                        printf("TA #%d modified question %d on exam %s for student %04d as marked!\n",
                               i + 1,
                               question_number_arr[exam_q_to_mark] + 1,
                               exam_file_name,
                               exam->student_number);

                        printf(ANSI_COLOR_RED "\n------------TA #%d (PID: %d) UNLOCKING SEMAPHORE (%d) FROM EXAM MARKING------------" ANSI_COLOR_RESET "\n", i + 1, getpid(), semaphore_id);
                        signalSemaphore(semaphore_id); // unlock semaphore from the specific process
                    }
                    // this is if the question is already marked, we remove that questions number from the question_number_arr[] array
                    // so it cannot be chosen in a future iteration for marking by this TA process
                    else
                    {
                        length_question_number_arr =
                            remove_by_index(question_number_arr,
                                            length_question_number_arr,
                                            exam_q_to_mark);

                        // if the length of the length_question_number_arr[] is 0, that means that every single question number has been removed,
                        // i.e., every single question has been marked, we can break from the while()
                        if (length_question_number_arr == 0)
                            break;
                    }
                }
                shm_unlink(exam_file_name); // remove name of shared memory object
            }
            // When every single exam in exam_files[] has been processed by this TA, we can exit this TA process
            exit(0);
        }
        else
        {
            ta_pids[i] = pid; // ensure the TA process PID get's addedd to the ta_pids array
        }
    }
    return ta_pids;
}

/**
 * @brief Decrement semaphore counter so only one process can work in critical section
 *
 * @param semaphoreID The ID of the semaphore we want to use
 */
void waitSemaphore(int semaphoreID)
{
    struct sembuf sem_op = {0, -1, 0};
    semop(semaphoreID, &sem_op, 1);
}

/**
 * @brief Increment semaphore counter so that another process can go work in the critical section
 *
 * @param semaphoreID The ID of the semaphore we want to use
 */
void signalSemaphore(int semaphoreID)
{
    struct sembuf sem_op = {0, 1, 0};
    semop(semaphoreID, &sem_op, 1);
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL); // print right away so nothing lags behind when checking printouts

    int number_of_tas; // initialize variable for number of ta's to be used later

    if (argc != 2)
    {
        printf("Number of required arguments was not supplied!\n");
        printf("Defaulting to 2 TA's\n");
        number_of_tas = 2;
    }
    else
    {
        number_of_tas = atoi(argv[1]);

        if (number_of_tas < 2)
        {
            printf("Must have at least two TA's!\n. Defaulting to 2...\n");
            number_of_tas = 2;
        }
    }

    // Create a semaphore set with SEM_KEY (20254001) that contains 1 semaphore
    int semaphore_id = semget(SEMAOPHORE_KEY, 1, IPC_CREAT | 0666);
    if (semaphore_id == -1)
    {
        fprintf(stderr, "Failed to create the semaphore set...\n");
        // we don't have to exit, part 2a showed the code can work without semaphore's, just may run into race conditions
    }

    // Initialize semaphore to 1 --> unlocked
    if (semaphore_id != -1)
    {
        if (semctl(semaphore_id, 0, SETVAL, 1) == -1)
        {
            fprintf(stderr, "Failed to initialize the semaphore (unlocked)...\n");
            // we don't have to exit, part 2a showed the code can work without semaphore's, just may run into race conditions
        }
    }

    srand(time(NULL));
    // *********************
    // on startup, load rubric and first exam into shared memory
    // create the rubric object in shared memory
    rubric_shared_data *rubric = createSharedMemRubric();
    if (!rubric)
    {
        fprintf(stderr, "Failed to create and/or map rubric in shared memory!\n");
        return 1;
    }
    // load rubric.txt into rubric shared memory object
    if (!load_rubric(rubric))
    {
        fprintf(stderr, "Failed to load rubric contents!\n");
        return 1;
    }
    // *********************

    int exam_count;
    char **exam_files = list_exams(&exam_count);

    exam_file_shared_data *exam = createSharedMemExam("exam1");
    if (!exam)
    {
        fprintf(stderr, "Failed to create and/or map rubric in shared memory!\n");
        exit(1);
    }

    pid_t *ta_process_pids = create_ta_processes(number_of_tas, rubric, exam, exam_count, exam_files, semaphore_id);

    // wait for all ta process to finish
    for (int i = 0; i < number_of_tas; i++)
    {
        waitpid(ta_process_pids[i], NULL, 0);
        printf(ANSI_COLOR_RED "\n------------TERMINATING TA PROCESS------------" ANSI_COLOR_RESET "\n");

        printf("TA %d with PID %d has terminated.\n", i + 1, ta_process_pids[i]);
    }
    free(ta_process_pids); // free the ta process pid array from memory
    free(exam_files);
    return 0;
}
