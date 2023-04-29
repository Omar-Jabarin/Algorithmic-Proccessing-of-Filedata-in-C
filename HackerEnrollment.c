#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "IsraeliQueue.h"


// Auxiliary Functions
int strToInt(char* str) {
    int rv = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9') {
            continue;
        }
        rv *= 10;
        rv += (int)(str[i]-'0');
    }
    return rv;
}

int asciiToInt(char* str, char mode) {
    int rv = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (mode == 'l' && 'A' <= str[i] && str[i] <= 'Z') {
            rv += ('A' - 'a');
        }
        rv += (int)str[i];
    }
    return rv;
}

char* copyStr(char* str) {
    char* copy = malloc(sizeof(char)*strlen(str));
    strcpy(copy, str);
    return copy;
}

int getNumRows(FILE* fp) {
    int num_rows = 1;

    char c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            num_rows++;
        }
        c = fgetc(fp);
    }

    fseek(fp, 0, SEEK_SET);
    return num_rows;
}

int getMaxRowLen(FILE* fp) {
    int max_len = 0;
    int temp = 0;

    char c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            max_len = (max_len > temp) ? max_len : temp;
            temp = 0;
        } else {
            temp++;
        }
        c = fgetc(fp);
    }

    fseek(fp, 0, SEEK_SET);
    return max_len+2; // for some reason createStudents works only if I add 2 here
}


// ValType IMPL
typedef union {
    int i;
    void* ptr;
} ValType;

ValType parseValType(char* ptr, int (*func)(char*)) {
    ValType val;
    if (func == NULL) {
        val.ptr = copyStr(ptr);
    } else {
        val.i = func(ptr);
    }
    return val;
}

// LinkedList IMPL
typedef struct LinkedList {
    ValType val;
    struct LinkedList* next;
} LinkedList;

LinkedList* initNode(ValType val) {
    LinkedList* node = malloc(sizeof(LinkedList));
    node->next = NULL;
    node->val = val;
    return node;
}

void* destroyLinkedList(LinkedList* node, char mode) {
    if (node == NULL) {
        return NULL;
    }
    if (node->next != NULL) {
        destroyLinkedList(node->next, mode);
    }
    if (mode == 'p') {
        free(node->val.ptr);
    }
    free(node->next);
    return NULL;
}

LinkedList* pushLinkedList(LinkedList* node, ValType val) {
    if (node == NULL) {
        return initNode(val);
    }
    while (node->next != NULL) {
        node = node->next;
    }
    node->next = initNode(val);
    return node->next;
}

LinkedList* parseLinkedList(char* str, int(*func)(char*)) {
    char* token;
    char delim[] = " ";

    token = strtok(str, delim);
    if (token == NULL) { // Empty string
        return NULL;
    }

    LinkedList* node = initNode(parseValType(token, func));
    LinkedList* initial = node;

    token = strtok(NULL, delim);
    while (token != NULL) {
        node->next = initNode(parseValType(token, func));
        node = node->next;
        token = strtok(NULL, delim);
    }
    
    return initial;
}

void printValType(ValType val, const char* format, char mode) {
    if (mode == 'p') {
        printf(format, val.ptr);
    } else {
        printf(format, val.i);
    }
}

void printLinkedList(LinkedList* node, const char* format, const char* sep, char mode) {
    while (node->next != NULL) {
        printValType(node->val, format, mode);
        printf(sep);
        node = node->next;
    }
    printValType(node->val, format, mode);
    printf("\n");
}

void printReversedLinkedList(LinkedList* node, const char* format, const char* sep, char mode) {
    if (node->next == NULL) {
        printValType(node->val, format, mode);
    } else {
        printReversedLinkedList(node->next, format, sep, mode);
        printf(sep);
        printValType(node->val, format, mode);
    }
}

bool contains(LinkedList* node, int val) {
    if (node == NULL) {
        return false;
    }
    while (node->val.i != val) {
        if (node->next == NULL) {
            return false;
        }
        node = node->next;
    }
    return true;
}

// Classes IMPL
typedef struct {
    LinkedList* desired_courses;
    LinkedList* friends;
    LinkedList* rivals;
    int failed_courses;
} HackerProfile;

typedef struct {
    int id;
    int ascii_upper;
    int ascii_lower;
    HackerProfile* profile;
} Student;

typedef struct {
    int id;
    int size;
    IsraeliQueue queue;
} Course;

Student* createStudent(char* str) {
    Student* ptr = malloc(sizeof(Student));

    char* token;
    char delim[] = " ";
    token = strtok(str, delim);
    ptr->id = strToInt(token);

    for (int i = 0; i < 3; i++) {
        token = strtok(NULL, delim);
    }

    char* name = malloc(sizeof(char)*strlen(token));
    if (!name) {
        exit(1);
    }
    
    strcpy(name, token);
    token = strtok(NULL, delim);
    char* surname = malloc(sizeof(char)*strlen(token));
    if (!surname) {
        exit(1);
    }
    
    strcpy(surname, token);
    ptr->ascii_upper = asciiToInt(name, 'u') + asciiToInt(surname, 'u');
    ptr->ascii_lower = asciiToInt(name, 'l') + asciiToInt(surname, 'l');
    ptr->profile = NULL;

    free(name);
    free(surname);

    return ptr;
}

Course* createCourse(char* str) {
    Course* ptr = malloc(sizeof(Course));
    
    char *token;
    char delim[] = " ";

    token = strtok(str, delim);
    ptr->id = strToInt(token);
    
    token = strtok(NULL, delim);
    ptr->size = strToInt(token);
    ptr->queue = NULL;
    return ptr;
}

Student* findStudent(LinkedList* students, int id) {
    while (((Student *)(students->val.ptr))->id != id) {
        if (students->next == NULL) {
            return NULL;
        }
        students = students->next;
    }
    return students->val.ptr;
}

void createProfile(LinkedList* students, char* buffer, const int BUFFER_SIZE, FILE* fp) {
    HackerProfile* ptr = malloc(sizeof(HackerProfile));
    Student* hacker = findStudent(students, strToInt(buffer));
    
    fgets(buffer, BUFFER_SIZE, fp);
    ptr->desired_courses = parseLinkedList(buffer, strToInt);
    fgets(buffer, BUFFER_SIZE, fp);
    ptr->friends = parseLinkedList(buffer, strToInt);
    fgets(buffer, BUFFER_SIZE, fp);
    ptr->rivals = parseLinkedList(buffer, strToInt);
    ptr->failed_courses = 0;

    hacker->profile = ptr;
}

void updateHackers(LinkedList* students, FILE* fp) {
    const int BUFFER_SIZE = getMaxRowLen(fp);
    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        createProfile(students, buffer, BUFFER_SIZE, fp);
    }
}

LinkedList* fileToLinkedList(FILE* fp, char mode) {
    const int BUFFER_SIZE = getMaxRowLen(fp);
    LinkedList* newList = NULL;
    LinkedList* initial = NULL;
    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        ValType temp;
        switch (mode) {
            case 's':
                // printf("yo");
                temp.ptr = createStudent(buffer);
                // printf("%d", ((Student *)(temp.ptr))->id);
                break;
            case 'c':
                temp.ptr = createCourse(buffer);
                break;
        }
        newList = pushLinkedList(newList, temp);
        if (initial == NULL) {
            initial = newList;
        }
    }
    free(buffer);
    return initial;
}

void printStudent(Student* student) {
    printf("%d-%d-%d-", student->id, student->ascii_upper, student->ascii_lower);
    if (student->profile == NULL) {
        printf("false\n");
    } else {
        printf("true\n");
        printLinkedList(student->profile->desired_courses, "%d", "-", 'i');
        printLinkedList(student->profile->friends, "%d", "-", 'i');
        printLinkedList(student->profile->rivals, "%d", "-", 'i');
    }
}

void printCourse(Course* course) {
    printf("%d-%d-", course->id, course->size);
    if (course->queue == NULL) {
        printf("false\n");
    } else {
        printf("true\n");
    }
}

void printLinkedListClass(LinkedList* node, void (*print_func)(void *)) {
    while (node != NULL) {
        print_func(node->val.ptr);
        node = node->next;
    }
}


typedef int (*f_ptr)(void*, void*);
// Friendship Functions
int comparisonFunction(Student *a, Student *b) {
    return (a->id == b->id);
}

int friendshipFunctionID (Student *a, Student *b) {
    if (a->profile == NULL && b->profile == NULL) {
        return 0;
    }
    return abs(a->id - b->id);
}

int friendshipFunctionUppercase (Student *a, Student *b) {
    if (a->profile == NULL && b->profile == NULL) {
        return 0;
    }
    return abs(a->ascii_upper - b->ascii_upper);
}

int friendshipFunctionLowercase (Student *a, Student *b) {
    if (a->profile == NULL && b->profile == NULL) {
        return 0;
    }
    return abs(a->ascii_lower - b->ascii_lower);
}

int friendshipRivalryFunction(Student* a, Student* b) {
    const int FRIENDSHIP_RV = 20;
    const int RIVALRY_RV = -20;
    if (a->profile != NULL) {
        if (contains(a->profile->friends, b->id)) {
            return FRIENDSHIP_RV;
        }
        if (contains(a->profile->rivals, b->id)) {
            return RIVALRY_RV;
        }
    }
    if (b->profile != NULL) {
        if (contains(b->profile->friends, a->id)) {
            return FRIENDSHIP_RV;
        }
        if (contains(b->profile->rivals, a->id)) {
            return RIVALRY_RV;
        }
    }
    return 0;
}

f_ptr* createFriendshipFunctions(char mode) {
    f_ptr* friendship_functions = malloc(4*sizeof(f_ptr*));
    if (!friendship_functions) {
        return NULL;
    }
    friendship_functions[0] = (f_ptr)friendshipFunctionID;
    friendship_functions[1] = (mode == 'u') ? (f_ptr)friendshipFunctionUppercase : (f_ptr)friendshipFunctionLowercase;
    friendship_functions[2] = (f_ptr)friendshipRivalryFunction;
    friendship_functions[3] = NULL;

    return friendship_functions;
}


// EnrollmentSystem IMPL
typedef struct {
    LinkedList* courses;
    LinkedList* students;
    f_ptr* friendship_functions;
} EnrollmentSystem_t, *EnrollmentSystem;

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers) {
    EnrollmentSystem sys = malloc(sizeof(EnrollmentSystem_t));

    sys->students = fileToLinkedList(students, 's');
    sys->courses = fileToLinkedList(courses, 'c');
    updateHackers(sys->students, hackers);
    sys->friendship_functions = NULL;
    
    return sys;
}

void initQueues(EnrollmentSystem sys, char mode) {
    const int FRIENDSHIP_TH = 20;
    const int RIVALRY_TH = -10;
    sys->friendship_functions = createFriendshipFunctions(mode);
    LinkedList* courses = sys->courses;
    while (courses != NULL) {
        ((Course *)(courses->val.ptr))->queue = IsraeliQueueCreate(sys->friendship_functions, (f_ptr)comparisonFunction, FRIENDSHIP_TH, RIVALRY_TH);
        courses = courses->next;
    }
}

void parseQueue(char* str, LinkedList* courses, LinkedList* students) {
    char* token;
    char delim[] = " ";

    token = strtok(str, delim);
    int id = strToInt(token);

    while (((Course *)(courses->val.ptr))->id != id) {
        courses = courses->next;
    }

    token = strtok(NULL, delim);
    while (token != NULL) { // tries to enqueue NULL
        IsraeliQueueEnqueue(((Course *)(courses->val.ptr))->queue, findStudent(students, strToInt(token)));
        token = strtok(NULL, delim);
    }
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queue) {
    const int BUFFER_SIZE = getMaxRowLen(queue);

    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    while (fgets(buffer, BUFFER_SIZE, queue) != NULL) {
        parseQueue(buffer, sys->courses, sys->students);
    }
    return sys;
}

void printEnrollmentSystem(EnrollmentSystem sys) {
    printf("Students: \n");
    printLinkedListClass(sys->students, (void (*)(void *)) printStudent);
    printf("\nCourses: \n");
    printLinkedListClass(sys->courses, (void (*)(void *)) printCourse);
    printf("Finished.\n");
}

void addHackerToCourses(LinkedList* courses, Student* hacker) {
    while (courses != NULL) {
        if ((contains(hacker->profile->desired_courses, ((Course *)(courses->val.ptr))->id))\
            && IsraeliQueueContains(((Course *)(courses->val.ptr))->queue, hacker) == 0) {
            IsraeliQueueEnqueue(((Course *)(courses->val.ptr))->queue, hacker);
        }
        courses = courses->next;
    }
}

int writeCourseToFile(FILE* fp, char* buffer, Course* course) {
    int len_queue = IsraeliQueueSize(course->queue);
    Student* temp;
    for (int i = 0; i < len_queue; i++) {
        temp = IsraeliQueueDequeue(course->queue);
        sprintf(buffer, "%d", temp->id);
        fputs(buffer, fp);
        fputs(" ", fp);
    }
    fputs("\n", fp);
    return 0;
}

Student* testHackerPositionQueues(Course* course, LinkedList* students) {
    int queue_len = IsraeliQueueSize(course->queue);
    IsraeliQueue queue_clone = IsraeliQueueClone(course->queue);
    Student* temp;
    for (int i = 1; i <= queue_len; i++) {
        temp = IsraeliQueueDequeue(queue_clone);
        if (i >= course->size && temp->profile != NULL) {
            (temp->profile->failed_courses)++;
            if (temp->profile->failed_courses >= 2){
                return temp;
            }
        }
    }
    return NULL;
}

Student* testHackerPositions(LinkedList* courses, LinkedList* students) {
    while (courses != NULL) {
        Student* hackerFailed = testHackerPositionQueues(((Course *)courses->val.ptr), students);
        if (hackerFailed != NULL) {
            return hackerFailed;
        }
        courses = courses->next;
    }
    return NULL;
}


void hackEnrollment(EnrollmentSystem sys, FILE* out) {
    const int BUFFER_SIZE = 20; //contains int in str format
    LinkedList* students = sys->students;
    while (students != NULL) {
        if (((Student *)(students->val.ptr))->profile != NULL) {
            addHackerToCourses(sys->courses, (Student *)(students->val.ptr));
        }
        students = students->next;
    }
    LinkedList* courses = sys->courses;
    while (courses != NULL) {
        IsraeliQueueImprovePositions(((Course *)(courses->val.ptr))->queue);
        courses = courses->next;
    }
    Student* hackerFailed = testHackerPositions(sys->courses, sys->students);
    if (hackerFailed != NULL) {
        fprintf(out, "Cannot satisfy constraints for %d", hackerFailed->id);
        return;
    }
    
    courses = sys->courses;
    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    while (courses != NULL) {
        writeCourseToFile(out, buffer, (Course *)(courses->val.ptr));
        courses = courses->next;
    }
    free(buffer);
}

void destroyEnrollment(EnrollmentSystem sys) {
    LinkedList* students = sys->students;
    while (students != NULL) {
        if (((Student *)(students->val.ptr))->profile != NULL) {
            destroyLinkedList(((Student *)(students->val.ptr))->profile->desired_courses, 'i');
            destroyLinkedList(((Student *)(students->val.ptr))->profile->friends, 'i');
            destroyLinkedList(((Student *)(students->val.ptr))->profile->rivals, 'i');
        }
        students = students->next;
    }
    destroyLinkedList(sys->students, 'p');
    LinkedList* courses = sys->courses;
    while (courses != NULL) {
        IsraeliQueueDestroy(((Course *)(courses->val.ptr))->queue);
        courses = courses->next;
    }
    destroyLinkedList(sys->courses, 'p');
    free(sys->friendship_functions);
    free(sys);
}




// int main() {
//     FILE* students = fopen("students.txt","r");
//     FILE* courses = fopen("courses.txt", "r");
//     FILE* hackers = fopen("hackers.txt", "r");

//     EnrollmentSystem sys = createEnrollment(students, courses, hackers);
//     initQueues(&sys, 'u'); // STOPS HERE
//     FILE* queues = fopen("queues.txt", "r");
//     readEnrollment(sys, queues);
//     FILE* target = fopen("out.txt", "w");
//     hackEnrollment(sys, target);
//     FILE* flag = fopen("flag.txt", "w");
//     fputs("FINISHED", flag);
// }