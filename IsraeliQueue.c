#include <stdlib.h>
#include <stdio.h>
#include "IsraeliQueue.h"
#include <assert.h>
#include <string.h>
#define UNINITIALIZED -1

typedef enum { FRIEND, RIVAL, NEUTRAL } Relation;

typedef struct{
	int friendship_quota;
	int rivalry_quota;
	void* object;
} Quotas;

Quotas* CopyQuotas(Quotas* quotas, int size){
	Quotas* newquotas = malloc(size*sizeof(Quotas));
	if (newquotas==NULL){
	#ifndef DNDEBUG
	printf("CopyQuotas: malloc failed in CopyQuotas");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newquotas[i]=quotas[i];
	}
	return newquotas;
}

struct IsraeliQueue_t {
	int friendship_th;
	int rivalry_th;
	FriendshipFunction* friendshipfunctions;
	ComparisonFunction compare;
	void** objects;
	int size;
	Quotas* quotas;
};


Relation CheckRelation( FriendshipFunction* friendshipfunctions,
 void* object1, void* object2, int friendship_th, int rivalry_th ){
	
	Relation current_relation=NEUTRAL;
	int sum_of_friendship_pts=0;
	int count=0;
	while (friendshipfunctions!=NULL && *friendshipfunctions!=NULL){
		if ((*friendshipfunctions)(object1, object2)>friendship_th){
			current_relation=FRIEND;
			return current_relation;
		}

		sum_of_friendship_pts+=(*friendshipfunctions)(object1, object2);
		count++;
		friendshipfunctions++;
	}
	double average_friendship_points=0;

	if (count!=0){
	average_friendship_points=(double)sum_of_friendship_pts/count;
	}
	if (average_friendship_points<(double)rivalry_th){
		current_relation=RIVAL;
		return current_relation;
	}

	assert(current_relation==NEUTRAL);
	return current_relation;

	}

void** CopyObjectArray(void** objects, int size){
	void** newobjects = malloc(size*sizeof(void*));
	if (newobjects==NULL){
	#ifndef DNDEBUG
	printf("CopyObjectArray: malloc failed in CopyObjectArray");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newobjects[i]=objects[i];
	}
	return newobjects;
}


IsraeliQueue IsraeliQueueCreate(FriendshipFunction* friendshipfunctions, ComparisonFunction compare, int friendship_th, int rivalry_th){

if (friendshipfunctions==NULL || compare==NULL){

#ifndef DNDEBUG
printf("IsraeliQueueCreate: friendshipfunctions or compare is NULL");
#endif

	return NULL;

}

IsraeliQueue newqueue = malloc(sizeof(*newqueue));
if (newqueue==NULL){

#ifndef DNDEBUG
printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
#endif

	return NULL;
}


newqueue->friendship_th=friendship_th;
newqueue->rivalry_th=rivalry_th;
newqueue->friendshipfunctions=friendshipfunctions;
newqueue->compare=compare;
newqueue->objects=malloc(sizeof(void*));
if (newqueue->objects==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		free(newqueue);
		return NULL;
}
newqueue->objects[0]=NULL;
newqueue->size=1;
newqueue->quotas=NULL;
return newqueue;

}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q){
	
	if (q==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueClone: q is NULL");
	#endif

	return NULL;
	}

	IsraeliQueue newqueue = malloc(sizeof(*newqueue));
	if (newqueue==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		return NULL;
	}

	newqueue->friendship_th=q->friendship_th;
	newqueue->rivalry_th=q->rivalry_th;
	newqueue->friendshipfunctions=q->friendshipfunctions;
	newqueue->compare=q->compare;
	newqueue->objects=CopyObjectArray(q->objects, q->size);
	newqueue->quotas=CopyQuotas(q->quotas, q->size);
	newqueue->size=q->size;

	return newqueue;
}


void IsraeliQueueDestroy(IsraeliQueue q){
	if (q==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueDestroy: q is NULL");
	#endif
		return;
	}

	if (q->objects!=NULL){
		free(q->objects);
	}
	if (q->quotas!=NULL){
		free(q->quotas);
	}
		free(q);
	return;
}





IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue q, void* object){
	if (q==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: q is NULL");
	#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	if (object==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: object is NULL");
	#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}


	int friend_location=UNINITIALIZED;
	bool skipped_to_friend=false;
	bool blocked=false;

	for (int i=0; i<((q->size)-1); i++){
	friend_location=UNINITIALIZED;
	skipped_to_friend=false;
	blocked=false;

		if(CheckRelation(q->friendshipfunctions, q->objects[i], object, q->friendship_th, q->rivalry_th)==FRIEND){
			if (q->quotas[i].friendship_quota<FRIEND_QUOTA){
				for(int j=i+1; j<((q->size)-1);  j++){
					if (CheckRelation(q->friendshipfunctions, q->objects[j], object, q->friendship_th, q->rivalry_th)==RIVAL){
						if (q->quotas[j].rivalry_quota<RIVAL_QUOTA){
							q->quotas[j].rivalry_quota++;
							i=j;
							blocked=true; //j is the index of the current blocking rival
							break;
						}
					}
				}	
					if (!blocked){
					friend_location=i; //friend_location is the index of the first available friend who didn't exceed quota and cannot be blocked by a rival
					skipped_to_friend=true;
					q->quotas[friend_location].friendship_quota++;
					break;
					}
			}

		}
	}



		
	q->objects=realloc(q->objects, (q->size+1)*sizeof(void*));
	if (q->objects==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	q->size+=1;
	q->objects[q->size-1]=NULL;

	q->quotas=realloc(q->quotas, (q->size+1)*sizeof(Quotas));
	if (q->quotas==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}

	if (skipped_to_friend){
		for (int i=q->size-1; i>friend_location; i--){
			q->objects[i]=q->objects[i-1];
			q->quotas[i]=q->quotas[i-1];
		}
		q->objects[friend_location+1]=object;
		q->quotas[friend_location+1].friendship_quota=0;
		q->quotas[friend_location+1].rivalry_quota=0;
		q->quotas[friend_location+1].object=object;
	}
	else{
		q->objects[q->size-2]=object;
		q->quotas[q->size-2].friendship_quota=0;
		q->quotas[q->size-2].rivalry_quota=0;
		q->quotas[q->size-2].object=object;
	}
		q->quotas[q->size-1].object=NULL;
	return ISRAELIQUEUE_SUCCESS;
	}

void* IsraeliQueueDequeue(IsraeliQueue q){
	if (q==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: q is NULL");
		#endif
		return NULL;
	}

	if (q->objects[0]==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: q is empty");
		#endif
		return NULL;
	}

	void* object=q->objects[0];
	for (int i=0; i<q->size-1; i++){
		q->objects[i]=q->objects[i+1];
		q->quotas[i]=q->quotas[i+1];
	}
	q->objects=realloc(q->objects, (q->size-1)*sizeof(void*));
	if (q->objects==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: realloc failed in IsraeliQueueDequeue");
		#endif
		return NULL;
	}
	q->size-=1;
	return object;

}

//test 3

int arr[]={7,6,5,4,3,2,1};

int MockFriendshipFunction(void* object1, void* object2){// 8 and 5
	if ((*(int*)object1==7 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==7)){
		return 9;
	}

	if ((*(int*)object1==6 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==6)){
		return 4;
	}
	if ((*(int*)object1==5 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==5)){
		return 4;
	}

	if ((*(int*)object1==4 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==4)){
		return 9;
	}
	if ((*(int*)object1==3 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==3)){
		return 4;
	}

	if ((*(int*)object1==2 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==2)){
		return 9;
	}
	if ((*(int*)object1==1 && *(int*)object2==8) || (*(int*)object1==8 && *(int*)object2==1)){
		return 9;
	}

		return 6; //6 is neutral 4 is rival and 9 is friend


}

int comparison_function_mock(void *obj1, void *obj2) {
    int id1 = *(int *)obj1;
    int id2 = *(int *)obj2;

    return id1 - id2;
}

FriendshipFunction friendshipfunctions[]={MockFriendshipFunction, MockFriendshipFunction, NULL};

int eight_value = 8;
int *eight = &eight_value;

void PrintIsraeliQueue(IsraeliQueue q) {
    if (q == NULL) {
        printf("Invalid queue.\n");
        return;
    }

    printf("IsraeliQueue content:\n");
    for (int i = 0; i < q->size - 1; i++) {
        printf("%d ", *((int *)q->objects[i]));
    }
    printf("\n");
}


void Test3(){
	


	IsraeliQueue q=IsraeliQueueCreate(friendshipfunctions, comparison_function_mock, 8, 5);
	for (int i=0; i<7; i++){
		IsraeliQueueEnqueue(q, &arr[i]);
	}
	IsraeliQueueEnqueue(q, eight);
	IsraeliQueue p=IsraeliQueueClone(q);

	int* out=(int*)IsraeliQueueDequeue(q);
	PrintIsraeliQueue(p);
	PrintIsraeliQueue(q);
	printf("out is %d", *out);




}

int main(){
	Test3();
	return 0;
}

/*
//test 2


typedef struct {
    int age;
    char* name;
    char* hobbies[5];
} Person;

// A friendship function based on age difference
int AgeDifferenceFriendship(void* object1, void* object2) {
    Person* person1 = (Person*)object1;
    Person* person2 = (Person*)object2;

    int age_difference = abs(person1->age - person2->age);
    return 100 - age_difference * 2;
}

// A friendship function based on shared hobbies
int SharedHobbiesFriendship(void* object1, void* object2) {
    Person* person1 = (Person*)object1;
    Person* person2 = (Person*)object2;

    int shared_hobbies = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (strcmp(person1->hobbies[i], person2->hobbies[j]) == 0) {
                shared_hobbies++;
            }
        }
    }

    return shared_hobbies * 20;
}

void Test_CheckRelation() {
    Person alice = {25, "Alice", {"hiking", "reading", "movies", "cooking", "traveling"}};
    Person bob = {27, "Bob", {"hiking", "music", "movies", "cooking", "traveling"}};
    Person charlie = {35, "Charlie", {"gaming", "music", "art", "photography", "traveling"}};
    Person dave = {20, "Dave", {"sports", "gaming", "reading", "cooking", "traveling"}};

    FriendshipFunction friendship_functions[] = {AgeDifferenceFriendship, SharedHobbiesFriendship, NULL};

    int friendship_threshold = 85;
    int rivalry_threshold = 60;

    Relation result = CheckRelation(friendship_functions, &alice, &bob, friendship_threshold, rivalry_threshold);
    printf("Test 1: Alice and Bob - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));


    result = CheckRelation(friendship_functions, &alice, &charlie, friendship_threshold, rivalry_threshold);
    printf("Test 2: Alice and Charlie - Result: %s, Expected: RIVAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));


    result = CheckRelation(friendship_functions, &alice, &dave, friendship_threshold, rivalry_threshold);
    printf("Test 3: Alice and Dave - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = CheckRelation(friendship_functions, &bob, &charlie, friendship_threshold, rivalry_threshold);
    printf("Test 4: Bob and Charlie - Result: %s, Expected: NEUTRAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = CheckRelation(friendship_functions, &bob, &dave, friendship_threshold, rivalry_threshold);
    printf("Test 5: Bob and Dave - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = CheckRelation(friendship_functions, &charlie, &dave, friendship_threshold, rivalry_threshold);
    printf("Test 6: Charlie and Dave - Result: %s, Expected: RIVAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));
}

int main() {
    Test_CheckRelation();
    return 0;
}

	
 test 1

// Mock FriendshipFunction implementation
int friendship_function_mock(void *obj1, void *obj2) {
    int id1 = *(int *)obj1;
    int id2 = *(int *)obj2;

    return abs(id1 - id2);
}

// Mock ComparisonFunction implementation
int comparison_function_mock(void *obj1, void *obj2) {
    int id1 = *(int *)obj1;
    int id2 = *(int *)obj2;

    return id1 - id2;
}



 int main() {
    // Create an array of FriendshipFunction pointers and initialize it with the mock function
    FriendshipFunction friendship_functions[] = {friendship_function_mock, NULL};

    // Create a new IsraeliQueue
    IsraeliQueue q = IsraeliQueueCreate(friendship_functions, comparison_function_mock, 2, 3);

    if (!q) {
        printf("Error: Failed to create IsraeliQueue\n");
        return 1;
    }

    // Create and enqueue sample objects (integers in this case)
    for (int i = 0; i < 5; i++) {
        int *new_object = malloc(sizeof(int));
        *new_object = i;
        IsraeliQueueError err = IsraeliQueueEnqueue(q, new_object);

        if (err != ISRAELIQUEUE_SUCCESS) {
            printf("Error: Failed to enqueue object %d\n", i);
            return 1;
        }
    }

    // Display the queue
    printf("Queue after enqueuing objects:\n");
    for (int i = 0; i < q->size; i++) {
        int *obj = (int *)q->objects[i];
        printf("Object ID: %d, Friendship Quota: %d, Rivalry Quota: %d\n", *obj, q->quotas[i].friendship_quota, q->quotas[i].rivalry_quota);
    }

    // Clone the queue
    IsraeliQueue cloned_q = IsraeliQueueClone(q);

    if (!cloned_q) {
        printf("Error: Failed to clone IsraeliQueue\n");
        return 1;
    }

    // Display the cloned queue
    printf("\nCloned queue:\n");
    for (int i = 0; i < cloned_q->size; i++) {
        int *obj = (int *)cloned_q->objects[i];
        printf("Object ID: %d, Friendship Quota: %d, Rivalry Quota: %d\n", *obj, cloned_q->quotas[i].friendship_quota, cloned_q->quotas[i].rivalry_quota);
    }

    // Cleanup
    for (int i = 0; i < q->size; i++) {
        free(q->objects[i]);
    }

    IsraeliQueueDestroy(q);
    IsraeliQueueDestroy(cloned_q);

    return 0;
} */