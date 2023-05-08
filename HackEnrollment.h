#ifndef PROVIDED_HACKENROLLMENT_H
#define PROVIDED_HACKENROLLMENT_H

typedef struct EnrollmentSystem_t *EnrollmentSystem;

// Reads files and creates an enrollment system struct.
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

// Reads file and updates the queues of the courses.
EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queue);

// Updates the friendship functions of all courses. If lower_flag is true then checks ascii distance as if the strings are lowercase.
void updateFriendshipFunction(EnrollmentSystem sys, int lower_flag);

// Updates the queues based on the hackers's desired courses.
void hackEnrollment(EnrollmentSystem sys, FILE* out);

// Frees all memory associated with the enrollment system.
void destroyEnrollment(EnrollmentSystem sys);

#endif
