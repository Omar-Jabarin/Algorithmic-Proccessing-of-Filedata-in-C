#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "HackerEnrollment.h"

EnrollmentSystem initEnrollment(char* students, char* courses, char* hackers) {
    FILE* fp_students = fopen(students, "r");
    FILE* fp_courses = fopen(courses, "r");
    FILE* fp_hackers = fopen(hackers, "r");
    if (!fp_students || !fp_courses || !fp_hackers) {
        fclose(fp_students);
        fclose(fp_courses);
        fclose(fp_hackers);
        return NULL;
    }
    
    EnrollmentSystem sys = createEnrollment(fp_students, fp_courses, fp_hackers);

    fclose(fp_students);
    fclose(fp_courses);
    fclose(fp_hackers);
    return sys;
}

int flagCheck(int argc, char** argv) {
    if (argc < 6 || argc > 7) {
        return -1;
    }

    if (argc == 7) {
        if (strcmp(argv[1],"-i") == 0) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    int flag = flagCheck(argc, argv);
    if (flag == -1) {
        return 1;
    }
    
    EnrollmentSystem sys = initEnrollment(argv[1 + flag], argv[2 + flag], argv[3 + flag]);    
    FILE* fp_queues = fopen(argv[4 + flag], "r");
    sys = readEnrollment(sys, fp_queues);
    FILE* fp_target = fopen(argv[5 + flag], "w");

    if (!fp_queues || !sys || !fp_target) {
        destroyEnrollment(sys);
        fclose(fp_queues);
        fclose(fp_target);
        return 1;
    }
    
    updateFriendshipFunction(sys, flag);
    hackEnrollment(sys, fp_target);

    fclose(fp_queues);
    fclose(fp_target);
    destroyEnrollment(sys);
    
    return 0;
}