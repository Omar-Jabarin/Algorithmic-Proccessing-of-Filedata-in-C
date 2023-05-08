#ifndef PROVIDED_HACKENROLLMENT_H
#define PROVIDED_HACKENROLLMENT_H

typedef struct EnrollmentSystem_t *EnrollmentSystem;

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queue);

void hackEnrollment(EnrollmentSystem sys, FILE* out);

void updateFriendshipFunction(EnrollmentSystem sys, int lower_flag);

void destroyEnrollment(EnrollmentSystem sys);

#endif
