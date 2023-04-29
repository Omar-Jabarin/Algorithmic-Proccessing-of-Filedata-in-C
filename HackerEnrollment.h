#ifndef PROVIDED_HACKERENROLLMENT_H
#define PROVIDED_HACKERENROLLMENT_H

typedef struct EnrollmentSystem_t *EnrollmentSystem;

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queue);

void hackEnrollment(EnrollmentSystem sys, FILE* out);

void initQueues(EnrollmentSystem sys, char mode);

void destroyEnrollment(EnrollmentSystem sys);

#endif