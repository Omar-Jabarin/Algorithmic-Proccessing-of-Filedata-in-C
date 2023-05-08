#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <IsraeliQueue.h>


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

char* copyStr(char* str) {
    if (!str || strlen(str) == 0) {
        return NULL;
    }
    char* copy = malloc(sizeof(char)*(strlen(str)+1));
    if (!copy) {
        return NULL;
    }
    strcpy(copy, str);
    return copy;
}

char getChar(char* str, int i, char mode) {
    if (i >= strlen(str)) {
        return 0;
    }
    if ('A' <= str[i] && str[i] <= 'Z' && mode == 'l') {
        return str[i] - 'A' + 'a';
    }
    return str[i];
}

int asciiDistance(char* str1, char* str2, char mode) {
    int rv = 0;
    for (int i = 0; i < fmax(strlen(str1), strlen(str2)); i++) {
        rv += abs(getChar(str1, i, mode) - getChar(str2, i, mode));
    }
    return rv;
}

int getNumRows(FILE* fp) {
    int counter = 1;

    char c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            counter++;
        }
        c = fgetc(fp);
    }

    fseek(fp, 0, SEEK_SET);
    return counter;
}

int getMaxRowLen(FILE* fp) {
    int maxLen = 0;
    int temp = 0;

    char c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            maxLen = (maxLen > temp) ? maxLen : temp;
            temp = 0;
        } else {
            temp++;
        }
        c = fgetc(fp);
    }
    maxLen = (maxLen > temp) ? maxLen : temp;

    fseek(fp, 0, SEEK_SET);
    return maxLen+2;
}


// ValType IMPL
typedef union {
    int i;
    void* ptr;
} ValType;

ValType parseValType(char* ptr, int (*func)(char*)) {
    ValType val;
    if (!func) {
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
    if (!node) {
        return NULL;
    }
    node->next = NULL;
    node->val = val;
    return node;
}

void destroyLinkedList(LinkedList* node) {
    if (!node) {
        return;
    }
    LinkedList* temp = node->next;
    free(node);
    destroyLinkedList(temp);
}

LinkedList* pushLinkedList(LinkedList* node, ValType val) {
    if (!node) {
        return initNode(val);
    }
    while (node->next) {
        node = node->next;
    }
    node->next = initNode(val);
    return node->next;
}

LinkedList* parseLinkedList(char* str, int(*func)(char*)) {
    char* token;
    char delim[] = " ";

    token = strtok(str, delim);
    if (!token) { // Empty string
        return NULL;
    }

    LinkedList* node = initNode(parseValType(token, func));
    if (!node) {
        return NULL;
    }
    LinkedList* initial = node;

    token = strtok(NULL, delim);
    while (token) {
        node->next = initNode(parseValType(token, func));
        if (!(node->next)) {
            return NULL;
        }
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
    while (node->next) {
        printValType(node->val, format, mode);
        printf(sep);
        node = node->next;
    }
    printValType(node->val, format, mode);
    printf("\n");
}

void printReversedLinkedList(LinkedList* node, const char* format, const char* sep, char mode) {
    if (!(node->next)) {
        printValType(node->val, format, mode);
    } else {
        printReversedLinkedList(node->next, format, sep, mode);
        printf(sep);
        printValType(node->val, format, mode);
    }
}

bool contains(LinkedList* node, int val) {
    if (!node) {
        return false;
    }
    while (node->val.i != val) {
        if (!(node->next)) {
            return false;
        }
        node = node->next;
    }
    return true;
}

int lenLinkedList(LinkedList* node) {
    int i = 0;
    while (node) {
        node = node->next;
        i++;
    }
    return i;
}


// Classes IMPL
typedef struct {
    LinkedList* desired_courses;
    LinkedList* friends;
    LinkedList* rivals;
} HackerProfile;

typedef struct {
    int id;
    char* name;
    char* surname;
    HackerProfile* profile;
} Student;

typedef struct {
    int id;
    int size;
    IsraeliQueue queue;
} Course;

Student* createStudent(char* str) {
    Student* ptr = malloc(sizeof(Student));
    if (!ptr) {
        return NULL;
    }

    char* token;
    char delim[] = " ";
    token = strtok(str, delim);
    ptr->id = strToInt(token);

    for (int i = 0; i < 3; i++) {
        token = strtok(NULL, delim);
    }

    ptr->name = copyStr(token);
    if (!(ptr->name)) {
        free(ptr);
        return NULL;
    }

    token = strtok(NULL, delim);
    ptr->surname = copyStr(token);
    if (!(ptr->surname)) {
        free(ptr->name);
        free(ptr);
        return NULL;
    }

    ptr->profile = NULL;

    return ptr;
}

void destroyStudent(Student* ptr) {
    if (ptr->profile) {
        destroyLinkedList(ptr->profile->desired_courses);
        destroyLinkedList(ptr->profile->friends);
        destroyLinkedList(ptr->profile->rivals);
    }
    free(ptr->name);
    free(ptr->surname);
    free(ptr->profile);
    free(ptr);
}

void destroyStudents(LinkedList* students) {
    LinkedList* temp = students;
    while (temp) {
        destroyStudent((Student *)(temp->val.ptr));
        temp = temp->next;
    }
    destroyLinkedList(students);
}

Course* createCourse(char* str) {
    Course* ptr = malloc(sizeof(Course));
    if (!ptr) {
        return NULL;
    }
    
    char *token;
    char delim[] = " ";

    token = strtok(str, delim);
    ptr->id = strToInt(token);
    
    token = strtok(NULL, delim);
    ptr->size = strToInt(token);
    ptr->queue = NULL;
    return ptr;
}

void destroyCourse(Course* ptr) {
    IsraeliQueueDestroy(ptr->queue);
    free(ptr);
}

void destroyCourses(LinkedList* courses) {
    LinkedList* temp = courses;
    while (temp) {
        destroyCourse((Course *)(temp->val.ptr));
        temp = temp->next;
    }
    destroyLinkedList(courses);
}

Student* findStudent(LinkedList* students, int id) {
    while (((Student *)(students->val.ptr))->id != id) {
        if (!(students->next)) {
            return NULL;
        }
        students = students->next;
    }
    return students->val.ptr;
}

void deleteNode(LinkedList* list, Student* ptr) {
    while (list->next) {
        if (list->next->val.ptr == ptr) {
            LinkedList* temp = list->next;
            list->next = temp->next;
            free(temp);
            return;
        }
        list = list->next;
    }
}

void pushHackerToEnd(LinkedList** students, Student* hacker) {
    if (((*students)->val.ptr) == hacker) {
        *students = (*students)->next;
    } else {
        deleteNode(*students, hacker);
    }
    ValType v;
    v.ptr = hacker;
    pushLinkedList(*students, v);
}

bool createProfile(LinkedList* students, char* buffer, const int BUFFER_SIZE, FILE* fp) {
    HackerProfile* ptr = malloc(sizeof(HackerProfile));
    Student* hacker = findStudent(students, strToInt(buffer));
    if (!ptr || !hacker) {
        return false;
    }
    pushHackerToEnd(&students, hacker);

    fgets(buffer, BUFFER_SIZE, fp);
    ptr->desired_courses = parseLinkedList(buffer, strToInt);
    fgets(buffer, BUFFER_SIZE, fp);
    ptr->friends = parseLinkedList(buffer, strToInt);
    fgets(buffer, BUFFER_SIZE, fp);
    ptr->rivals = parseLinkedList(buffer, strToInt);

    hacker->profile = ptr;
    return true;
}

bool updateHackers(LinkedList* students, FILE* fp) {
    const int BUFFER_SIZE = getMaxRowLen(fp);
    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    if (!buffer) {
        return false;
    }
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        createProfile(students, buffer, BUFFER_SIZE, fp);
    }
    free(buffer);
    return true;
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
                temp.ptr = createStudent(buffer);
                if (!temp.ptr) {
                    destroyStudents(initial);
                    return NULL;
                }
                break;
            case 'c':
                temp.ptr = createCourse(buffer);
                if (!temp.ptr) {
                    destroyCourses(initial);
                    return NULL;
                }
                break;
        }
        newList = pushLinkedList(newList, temp);
        if (!initial) {
            initial = newList;
        }
    }
    free(buffer);
    return initial;
}

void printStudent(Student* student) {
    printf("%d-%s-%s-", student->id, student->name, student->surname);
    if (!(student->profile)) {
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
    if (!(course->queue)) {
        printf("false\n");
    } else {
        printf("true\n");
    }
}

void printLinkedListClass(LinkedList* node, void (*printFunction)(void *)) {
    while (node) {
        printFunction(node->val.ptr);
        node = node->next;
    }
}


// Friendship Functions
typedef int (*f_ptr)(void*, void*);

int comparisonFunction(void* a, void* b) {
    Student* ta = (Student *) a;
    Student* tb = (Student *) b;
    return (ta->id == tb->id);
}

int friendshipFunctionID (void* a, void* b) {
    Student* ta = (Student *) a;
    Student* tb = (Student *) b;
    if (!(ta->profile) && !(tb->profile)) {
        return 0;
    }
    return abs(ta->id - tb->id);
}

int friendshipFunctionUppercase (void* a, void* b) {
    Student* ta = (Student *) a;
    Student* tb = (Student *) b;
    if (!(ta->profile) && !(tb->profile)) {
        return 0;
    }
    return asciiDistance(ta->name, tb->name, 'u') + asciiDistance(ta->surname, tb->surname, 'u');
}

int friendshipFunctionLowercase (void* a, void* b) {
    Student* ta = (Student *) a;
    Student* tb = (Student *) b;
    if (!(ta->profile) && !(tb->profile)) {
        return 0;
    }
    return asciiDistance(ta->name, tb->name, 'l') + asciiDistance(ta->surname, tb->surname, 'l');
}

int friendshipRivalryFunction(void* a, void* b) {
    Student* ta = (Student *) a;
    Student* tb = (Student *) b;
    const int FRIENDSHIP_RV = 20;
    const int RIVALRY_RV = -20;
    if (ta->profile) {
        if (contains(ta->profile->friends, tb->id)) {
            return FRIENDSHIP_RV;
        }
        if (contains(ta->profile->rivals, tb->id)) {
            return RIVALRY_RV;
        }
    }
    if (tb->profile) {
        if (contains(tb->profile->friends, ta->id)) {
            return FRIENDSHIP_RV;
        }
        if (contains(tb->profile->rivals, ta->id)) {
            return RIVALRY_RV;
        }
    }
    return 0;
}

// EnrollmentSystem IMPL
typedef struct {
    LinkedList* courses;
    LinkedList* students;
} EnrollmentSystem_t, *EnrollmentSystem;

EnrollmentSystem createEnrollment(FILE* students_fp, FILE* courses_fp, FILE* hackers_fp) {
    const int FRIENDSHIP_TH = 20;
    const int RIVALRY_TH = 0;
    EnrollmentSystem sys = malloc(sizeof(EnrollmentSystem_t));
    if (!sys) {
        return NULL;
    }

    sys->students = fileToLinkedList(students_fp, 's');
    if (!(sys->students)) {
        return NULL;
    }
    sys->courses = fileToLinkedList(courses_fp, 'c');
    if (!(sys->courses)) {
        destroyStudents(sys->students);
        return NULL;
    }
    if (updateHackers(sys->students, hackers_fp) == false) {
        destroyStudents(sys->students);
        destroyCourses(sys->courses);
        return NULL;
    }
    f_ptr* friendshipFunctions = malloc(sizeof(f_ptr));
    if (!friendshipFunctions) {
        destroyStudents(sys->students);
        destroyCourses(sys->courses);
        return NULL;
    }
    friendshipFunctions[0] = NULL;

    LinkedList* courses = sys->courses;
    while (courses) {
        ((Course *)(courses->val.ptr))->queue = IsraeliQueueCreate(friendshipFunctions, comparisonFunction, FRIENDSHIP_TH, RIVALRY_TH);
        if (!(((Course *)(courses->val.ptr))->queue)) {
            free(friendshipFunctions);
            return NULL;
        }
        courses = courses->next;
    }
    free(friendshipFunctions);
    return sys;
}

void updateFriendshipFunction(EnrollmentSystem sys, int lowerFlag) {
    if (!sys) {
        return;
    }
    LinkedList* course = sys->courses;
    while (course) {
        IsraeliQueueAddFriendshipMeasure(((Course *)(course->val.ptr))->queue, friendshipFunctionID);
        IsraeliQueueAddFriendshipMeasure(((Course *)(course->val.ptr))->queue, friendshipRivalryFunction);
        if (lowerFlag) {
            IsraeliQueueAddFriendshipMeasure(((Course *)(course->val.ptr))->queue, friendshipFunctionLowercase);
        } else {
            IsraeliQueueAddFriendshipMeasure(((Course *)(course->val.ptr))->queue, friendshipFunctionUppercase);
        }
        course = course->next;
    }
}

bool parseQueue(char* str, LinkedList* courses, LinkedList* students) {
    char* token;
    char delim[] = " ";

    token = strtok(str, delim);
    int id = strToInt(token);

    while (((Course *)(courses->val.ptr))->id != id) {
        courses = courses->next;
    }

    token = strtok(NULL, delim);
    while (token) {
        if (IsraeliQueueEnqueue(((Course *)(courses->val.ptr))->queue, findStudent(students, strToInt(token))) == ISRAELI_QUEUE_ERROR) {
            return false;
        }
        token = strtok(NULL, delim);
    }
    return true;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queue) {
    if (!sys) {
        return NULL;
    }
    const int BUFFER_SIZE = getMaxRowLen(queue);

    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);
    if (!buffer) {
        return NULL;
    }
    while (fgets(buffer, BUFFER_SIZE, queue)) {
        if (parseQueue(buffer, sys->courses, sys->students) == false) {
            free(buffer);
            return NULL;
        }
    }
    free(buffer);
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
    while (courses) {
        if (contains(hacker->profile->desired_courses, ((Course *)(courses->val.ptr))->id)) {
            IsraeliQueueEnqueue(((Course *)(courses->val.ptr))->queue, hacker);
        }
        courses = courses->next;
    }
}

void writeCourseToFile(FILE* fp, Course* course) {
    int lenQueue = IsraeliQueueSize(course->queue);
    if (lenQueue == 0) {
        return;
    }
    fprintf(fp, "%d", course->id);
    Student* temp;
    for (int i = 0; i < lenQueue; i++) {
        temp = IsraeliQueueDequeue(course->queue);
        fprintf(fp, " %d", temp->id);
    }
    fputs("\n", fp);
}

int testHackerPositionQueue(Student* hacker, Course* course) {
    if (!(contains(hacker->profile->desired_courses, course->id))) {
        return 0;
    }
    int lenQueue = IsraeliQueueSize(course->queue);
    IsraeliQueue queueClone = IsraeliQueueClone(course->queue);
    Student* temp;
    for (int i = 0; i < lenQueue; i++) {
        temp = IsraeliQueueDequeue(queueClone);
        if (temp->id == hacker->id) {
            IsraeliQueueDestroy(queueClone);
            return (i >= course->size);
        }
    }
    IsraeliQueueDestroy(queueClone);
    return 0;
}

int testHackerPosition(Student* hacker, LinkedList* courses) {
    if (!(hacker->profile)) {
        return 0;
    }
    if (!(hacker->profile->desired_courses)) {
        return 0;
    }
    int failed = 0;
    while (courses) {
        failed += testHackerPositionQueue(hacker, (Course *)courses->val.ptr);
        courses = courses->next;
    }
    int desired = lenLinkedList(hacker->profile->desired_courses);

    return (failed > desired - fmin(2, desired));
}

Student* testHackerPositions(LinkedList* courses, LinkedList* students) {
    while (students) {
        if (testHackerPosition(students->val.ptr, courses)) {
            return students->val.ptr;
        }
        students = students->next;
    }
    return NULL;

    // while (courses) {
    //     Student* hackerFailed = testHackerPositionQueues(((Course *)courses->val.ptr), students);
    //     if (hackerFailed) {
    //         return hackerFailed;
    //     }
    //     courses = courses->next;
    // }
    // return NULL;
}

void hackEnrollment(EnrollmentSystem sys, FILE* out) {
    LinkedList* students = sys->students;
    
    while (students) {
        if (((Student *)(students->val.ptr))->profile) {
            addHackerToCourses(sys->courses, (Student *)(students->val.ptr));
        }
        students = students->next;
    }
    
    Student* hackerFailed = testHackerPositions(sys->courses, sys->students);
    if (hackerFailed) {
        fprintf(out, "Cannot satisfy constraints for %d\n", hackerFailed->id);
        return;
    }
    
    LinkedList* courses = sys->courses;
    while (courses) {
        writeCourseToFile(out, (Course *)(courses->val.ptr));
        courses = courses->next;
    }
}

void destroyEnrollment(EnrollmentSystem sys) {
    if (sys) {
        destroyStudents(sys->students);
        destroyCourses(sys->courses);
        free(sys);
    }
}
