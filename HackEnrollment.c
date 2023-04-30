#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "HackerEnrollment.h"

EnrollmentSystem initEnrollment(char* students, char* courses, char* hackers, int flag) {
    FILE* fp_students = fopen(students, "r");
    if (!fp_students) {
        return NULL;
    }
    FILE* fp_courses = fopen(courses, "r");
    if (!courses) {
        fclose(fp_students);
        return NULL;
    }
    FILE* fp_hackers = fopen(hackers, "r");
    if (!fp_hackers) {
        fclose(fp_students);
        fclose(fp_courses);
        return NULL;
    }

    EnrollmentSystem new_sys = createEnrollment(fp_students, fp_courses, fp_hackers);

    char mode = (flag) ? 'l' : 'u';
    initQueues(new_sys, mode);

    fclose(fp_students);
    fclose(fp_courses);
    fclose(fp_hackers);

    return new_sys;
}

int main(int argc, char** argv) {
    if (argc < 6 || argc > 7) {
        return 1;
    }

    int flag = 0;
    if (argc == 7) {
        if (strcmp(argv[1],"-i") == 0) {
            flag = 1;
        } else {
            return 1;
        }
    }

    EnrollmentSystem new_sys = initEnrollment(argv[1 + flag], argv[2 + flag], argv[3 + flag], flag);
    FILE* fp_queues = fopen(argv[4 + flag], "r");
    if (!fp_queues) {
        return 1;
    }
    EnrollmentSystem sys = readEnrollment(new_sys, fp_queues);
    FILE* fp_target = fopen(argv[5 + flag], "w");
    if (!fp_target) {
        fclose(fp_queues);
        return 1;
    }

    hackEnrollment(sys, fp_target);
    fclose(fp_queues);
    fclose(fp_target);
    destroyEnrollment(sys);
    return 0;
}