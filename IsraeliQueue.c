#include <stdlib.h>
#include <stdio.h>
#include "IsraeliQueue.h"
#include <assert.h>
#include <string.h>
#include <math.h>

#define UNINITIALIZED -1
#define CANNOT_FIND -2
#define ENQUEUE_FAILED -3

typedef enum { FRIEND, RIVAL, NEUTRAL } Relation;

typedef struct{
	int m_friendshipQuota;
	int m_rivalryQuota;
	void* m_object;
	bool improved;
} Quota;

FriendshipFunction* copyFriendshipFunctions(FriendshipFunction* friendshipFunctions){
	int size=0;
	while (friendshipFunctions[size]!=NULL){
		size++;
	}
	FriendshipFunction* newFriendshipFunctions = malloc((size+1)*sizeof(FriendshipFunction));
	if (newFriendshipFunctions==NULL){
	#ifndef DNDEBUG
	printf("copyFriendshipFunctions: malloc failed in copyFriendshipFunctions");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newFriendshipFunctions[i]=friendshipFunctions[i];
	}
	newFriendshipFunctions[size]=NULL;
	return newFriendshipFunctions;

}
Quota* copyQuotas(Quota* quotas, int size){
	Quota* newQuotas = malloc(size*sizeof(Quota));
	if (newQuotas==NULL){
	#ifndef DNDEBUG
	printf("copyQuotas: malloc failed in copyQuotas");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newQuotas[i]=quotas[i];
	}
	return newQuotas;
}

struct IsraeliQueue_t {
	int m_friendshipThreshold;
	int m_rivalryThreshold;
	FriendshipFunction* m_friendshipFunctions;
	bool m_updatedFriendshipFunctions;
	ComparisonFunction m_compare;
	void** m_objects;
	int m_size;
	Quota* m_quotas;
};


Relation checkRelation(FriendshipFunction* friendshipFunctions,
 void* firstObject, void* secondObject, int friendshipThreshold, int rivalryThreshold ){
	
	Relation currentRelation=NEUTRAL;
	int sumOfFriendshipPoints=0;
	int count=0;
	while (friendshipFunctions!=NULL && *friendshipFunctions!=NULL){
		if ((*friendshipFunctions)(firstObject, secondObject)>friendshipThreshold){
			currentRelation=FRIEND;
			return currentRelation;
		}
		sumOfFriendshipPoints+=(*friendshipFunctions)(firstObject, secondObject);
		count++;
		friendshipFunctions++;
	}
	double averageFriendshipPoints=0;

	if (count!=0){
	averageFriendshipPoints=(double)sumOfFriendshipPoints/count;
	}
	if (averageFriendshipPoints<(double)rivalryThreshold){
		currentRelation=RIVAL;
		return currentRelation;
	}
	assert(currentRelation==NEUTRAL);
	return currentRelation;

	}

void** copyObjectArray(void** objects, int size){
	void** newObjects = malloc(size*sizeof(void*));
	if (newObjects==NULL){
	#ifndef DNDEBUG
	printf("copyObjectArray: malloc failed in copyObjectArray");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newObjects[i]=objects[i];
	}
	return newObjects;
}


IsraeliQueue IsraeliQueueCreate(FriendshipFunction *friendshipFunctions,
 								ComparisonFunction compare, int friendshipThreshold, int rivalryThreshold)
{

	if (friendshipFunctions == NULL || compare == NULL){
	#ifndef DNDEBUG
		printf("IsraeliQueueCreate: friendshipFunctions or compare is NULL");
	#endif

		return NULL;
	}

	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue == NULL){
	#ifndef DNDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		return NULL;
	}

	newQueue->m_friendshipThreshold = friendshipThreshold;
	newQueue->m_rivalryThreshold = rivalryThreshold;
	newQueue->m_friendshipFunctions = friendshipFunctions;
	newQueue->m_compare = compare;
	newQueue->m_updatedFriendshipFunctions = false;
	newQueue->m_objects = malloc(sizeof(void *));
	if (newQueue->m_objects == NULL){
	#ifndef DNDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		free(newQueue);
		return NULL;
	}

	newQueue->m_objects[0] = NULL;
	newQueue->m_size = 1;
	newQueue->m_quotas = NULL;
	return newQueue;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue queue){
	
	if (queue==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueClone: queue is NULL");
	#endif

	return NULL;
	}

	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		return NULL;
	}
	newQueue->m_friendshipThreshold=queue->m_friendshipThreshold;
	newQueue->m_rivalryThreshold=queue->m_rivalryThreshold;
	newQueue->m_friendshipFunctions=copyFriendshipFunctions(queue->m_friendshipFunctions);
	newQueue->m_compare=queue->m_compare;
	newQueue->m_objects=copyObjectArray(queue->m_objects, queue->m_size);
	newQueue->m_quotas=copyQuotas(queue->m_quotas, queue->m_size);
	newQueue->m_size=queue->m_size;
	newQueue->m_updatedFriendshipFunctions=true;
	return newQueue;
}


void IsraeliQueueDestroy(IsraeliQueue queue){
	if (queue==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueDestroy: queue is NULL");
	#endif
		return;
	}

	if (queue->m_objects!=NULL){
		free(queue->m_objects);
	}
	if (queue->m_quotas!=NULL){
		free(queue->m_quotas);
	}
	if (queue->m_updatedFriendshipFunctions){
		free(queue->m_friendshipFunctions);
	}
		free(queue);
	return;
}


void checkInsertLocation(IsraeliQueue queue, void* object, int *friendLocation,
														 bool *skippedToFriend){
	bool blocked=false;
		for (int i=0; i<((queue->m_size)-1); i++){
	*friendLocation=UNINITIALIZED;
	*skippedToFriend=false, blocked=false;
		if(checkRelation(queue->m_friendshipFunctions, queue->m_objects[i],
		 				object, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==FRIEND){
			if (queue->m_quotas[i].m_friendshipQuota<FRIEND_QUOTA){
				for(int j=i+1; j<((queue->m_size)-1);  j++){
					if (checkRelation(queue->m_friendshipFunctions, queue->m_objects[j],
					 object, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==RIVAL){
						if (queue->m_quotas[j].m_rivalryQuota<RIVAL_QUOTA){
							queue->m_quotas[j].m_rivalryQuota++;
							i=j;
							blocked=true; //j is the index of the current blocking rival
							break;
						}
					}
				}	
					if (!(blocked)){
					*friendLocation=i; 
					*skippedToFriend=true;
					queue->m_quotas[*friendLocation].m_friendshipQuota++;
					break;
					}
			}
		}
	}		
}


void insertObject(IsraeliQueue queue, void* object, int* friendLocation, bool* skippedToFriend, bool improving){
	if (*skippedToFriend){
		for (int i=queue->m_size-1; i>*friendLocation; i--){
			queue->m_objects[i]=queue->m_objects[i-1];
			queue->m_quotas[i]=queue->m_quotas[i-1];
		}
		queue->m_objects[*friendLocation+1]=object;
		queue->m_quotas[*friendLocation+1].m_friendshipQuota=0;
		queue->m_quotas[*friendLocation+1].m_rivalryQuota=0;
		queue->m_quotas[*friendLocation+1].m_object=object;
		if (!improving){
			queue->m_quotas[*friendLocation+1].improved=false;
		}
	}
	else{
		queue->m_objects[queue->m_size-2]=object;
		queue->m_quotas[queue->m_size-2].m_friendshipQuota=0;
		queue->m_quotas[queue->m_size-2].m_rivalryQuota=0;
		queue->m_quotas[queue->m_size-2].m_object=object;
		if (!improving){
			queue->m_quotas[queue->m_size-2].improved=false;
		}
	}
		queue->m_quotas[queue->m_size-1].m_object=NULL;

}


IsraeliQueueError resizeQueue(IsraeliQueue queue){
	queue->m_objects=realloc(queue->m_objects, (queue->m_size+1)*sizeof(void*));
	if (queue->m_objects==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	queue->m_size+=1;
	queue->m_objects[queue->m_size-1]=NULL;
	queue->m_quotas=realloc(queue->m_quotas, (queue->m_size+1)*sizeof(Quota));
	if (queue->m_quotas==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	return ISRAELIQUEUE_SUCCESS;
}

int enqueueAndGetIndex(IsraeliQueue queue, void* object){
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){					//Resizing
		return ENQUEUE_FAILED;
	}
	insertObject(queue, object, &friendLocation, &skippedToFriend, true);
	if (skippedToFriend){
		return friendLocation+1;
	}
	else{
		return queue->m_size-2;
	}
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* object){
	if (queue==NULL || object==NULL){
	#ifndef DNDEBUG
	printf("IsraeliQueueEnqueue: queue or object is NULL");
	#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){					//Resizing
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	insertObject(queue, object, &friendLocation, &skippedToFriend, false);
	return ISRAELIQUEUE_SUCCESS;
	}



void* IsraeliQueueDequeue(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: queue is NULL");
		#endif
		return NULL;
	}

	if (queue->m_objects[0]==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: queue is empty");
		#endif
		return NULL;
	}

	void* object=queue->m_objects[0];
	for (int i=0; i<queue->m_size-1; i++){
		queue->m_objects[i]=queue->m_objects[i+1];
		queue->m_quotas[i]=queue->m_quotas[i+1];
	}
	queue->m_objects=realloc(queue->m_objects, (queue->m_size-1)*sizeof(void*));
	if (queue->m_objects==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeue: realloc failed in IsraeliQueueDequeue");
		#endif
		return NULL;
	}
	queue->m_size-=1;
	return object;

}


bool IsraeliQueueContains(IsraeliQueue queue, void * object){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueContains: queue is NULL");
		#endif
		return false;
	}

	if (object==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueContains: object is NULL");
		#endif
		return false;
	}

	for (int i=0; i<queue->m_size-1; i++){
		if (queue->m_objects[i]==object){
			return true;
		}
	}
	return false;


}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction newFriendshipFunction) {
    if (queue == NULL || newFriendshipFunction == NULL) {
        #ifndef DNDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: queue or newFriendshipFunction is NULL");
        #endif
        return ISRAELIQUEUE_BAD_PARAM;
    }

    int count = 0;
    while (queue->m_friendshipFunctions[count] != NULL) {
        count++;
    }

    FriendshipFunction *newArray = malloc((count + 2) * sizeof(FriendshipFunction));
    if (newArray == NULL) {
        #ifndef DNDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: malloc failed");
        #endif
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    for (int i = 0; i < count; i++) {
        newArray[i] = queue->m_friendshipFunctions[i];
    }

    newArray[count] = newFriendshipFunction;
    newArray[count + 1] = NULL;
    queue->m_friendshipFunctions = newArray;
	queue->m_updatedFriendshipFunctions=true;

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueUpdateFriendshipThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_friendshipThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueUpdateRivalryThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_rivalryThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;

}

int IsraeliQueueSize(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueSize: queue is NULL");
		#endif
		return 0;
	}

	return queue->m_size-1;
}
void* IsraeliQueueDequeueAtIndex(IsraeliQueue queue, int index){
	if (queue == NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeueAtIndex: queue is NULL");
		#endif
		return NULL;
	}
	if (index < 0 || index >= queue->m_size - 1){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeueAtIndex: invalid index");
		#endif
		return NULL;
	}
	void* object = queue->m_objects[index];
	for (int i = index; i < queue->m_size - 1; i++){
		queue->m_objects[i] = queue->m_objects[i + 1];
		queue->m_quotas[i] = queue->m_quotas[i + 1];
	}
	void** tempObjects = realloc(queue->m_objects, (queue->m_size - 1) * sizeof(void*));
	if (tempObjects == NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueDequeueAtIndex: realloc failed in IsraeliQueueDequeueAtIndex");
		#endif
		return NULL;
	}
	queue->m_objects = tempObjects;
	queue->m_size -= 1;
	return object;
}




int FindIsraeliQueueObject(IsraeliQueue queue, void* object){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueFindObject: queue is NULL");
		#endif
		return CANNOT_FIND;
	}

	if (object==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueFindObject: object is NULL");
		#endif
		return CANNOT_FIND;
	}

	for (int i=0; i<queue->m_size-1; i++){
		if (queue->m_objects[i]==object){
			return i;
		}
	}
	return CANNOT_FIND;
}


IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueImprovePosition: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	IsraeliQueue originalQueue=IsraeliQueueClone(queue);
	if (originalQueue==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueImprovePosition: clone failed");
		#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}

	for (int i=originalQueue->m_size-2; i>=0; i--){
		void* currentObject=originalQueue->m_objects[i];
		for (int j=originalQueue->m_size-2; j>=0; j--){
			if ((currentObject==(queue->m_objects[j])) && !(queue->m_quotas[j].improved)){
				Quota currentQuota=queue->m_quotas[j];
				IsraeliQueueDequeueAtIndex(queue, j);
				int index=enqueueAndGetIndex(queue, currentObject);
				queue->m_quotas[index]=currentQuota; //retain quota
				queue->m_quotas[index].improved=true;
				break;

			}
		}

	}
	//reset improved flags
	for (int i=0; i<queue->m_size-1; i++){
		queue->m_quotas[i].improved=false;
	}
	IsraeliQueueDestroy(originalQueue);
	return ISRAELIQUEUE_SUCCESS;

}
/*
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue){
    if (queue==NULL){
        #ifndef DNDEBUG
        printf("IsraeliQueueImprovePosition: queue is NULL");
        #endif
        return ISRAELIQUEUE_BAD_PARAM;
    }

    // An array to keep track of the dequeued elements
    void** dequeuedElements = malloc(queue->m_size * sizeof(void*));
    if (dequeuedElements == NULL){
        #ifndef DNDEBUG
        printf("IsraeliQueueImprovePosition: malloc failed");
        #endif
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    int dequeuedCount = 0;
	Quota currentQuota=queue->m_quotas[queue->m_size-2];
    for (int i = queue->m_size - 2; i >= 0; i--) {
		currentQuota=queue->m_quotas[i];
        bool alreadyProcessed = false;
        // Check if the current element has been already dequeued and enqueued again
        for (int j = 0; j < dequeuedCount; j++) {
            if (queue->m_objects[i] == dequeuedElements[j]) {
                alreadyProcessed = true;
                break;
            }
        }
        // If the element has not been dequeued yet, dequeue and enqueue it again
        if (!alreadyProcessed) {
            void* object = IsraeliQueueDequeueAtIndex(queue, i);
            if (object == NULL) {
                free(dequeuedElements);
                return ISRAELI_QUEUE_ERROR;
            }
            dequeuedElements[dequeuedCount++] = object;
            IsraeliQueueEnqueue(queue, object);
			i=queue->m_size-2;
			int k=FindIsraeliQueueObject(queue, object);
			if (k==CANNOT_FIND){
				#ifndef DNDEBUG
				printf("IsraeliQueueImprovePosition: IsraeliQueueFindObject failed");
				#endif
				return ISRAELI_QUEUE_ERROR;
        	}
			queue->m_quotas[k]=currentQuota; //Retain the quota of dequeued and then enqueued element
		}
    }
    free(dequeuedElements);
    return ISRAELIQUEUE_SUCCESS;
}
*/

FriendshipFunction* getAllFriendshipFunctions(IsraeliQueue* queues) {
    int totalSize = 0;

    for (IsraeliQueue* queuePointer=queues; *queuePointer != NULL; queuePointer++) {
        for (FriendshipFunction* functionsPointer = (*queuePointer)->m_friendshipFunctions;
		 *functionsPointer != NULL; functionsPointer++) {
            
			totalSize++;
        }
    }

    FriendshipFunction* result = (FriendshipFunction*) malloc((totalSize + 1) * sizeof(FriendshipFunction));
    if (result == NULL) {
		#ifndef DNDEBUG
		printf("getAllFriendShipFunctions: malloc failed");
		#endif
        return NULL;
    }

    int currentIndex = 0;
    for (IsraeliQueue* queuePointer=queues; *queuePointer != NULL; queuePointer++) {
        for (FriendshipFunction* functionsPointer = (*queuePointer)->m_friendshipFunctions;
		 *functionsPointer != NULL; functionsPointer++) {
            result[currentIndex] = *functionsPointer;
            currentIndex++;
        }
    }

    result[currentIndex] = NULL;

    return result;
}


IsraeliQueue IsraeliQueueMerge(IsraeliQueue* queueArray, ComparisonFunction compare){
	if (queueArray==NULL || compare==NULL || *queueArray==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueMerge: queueArray or compare is NULL");
		#endif
		return NULL;
	}
	int count=0, sizeOfNewQueue=0, sumOfFriendshipThresholds=0, productOfRivalryThresholds=1;
	while (queueArray[count]!=NULL){
		sizeOfNewQueue+=(queueArray[count]->m_size-1);
		sumOfFriendshipThresholds+=queueArray[count]->m_friendshipThreshold;
		productOfRivalryThresholds*=queueArray[count]->m_rivalryThreshold;
		count++;
	}
	sizeOfNewQueue+=1;
	FriendshipFunction* friendshipFunctions=getAllFriendshipFunctions(queueArray);
	int averageFriendship=(int)ceil((double)sumOfFriendshipThresholds/count);
	int geometricMeanOfRivalry=(int)ceil(pow((double)productOfRivalryThresholds, 1.0/count));
	IsraeliQueue result=IsraeliQueueCreate(friendshipFunctions, compare, averageFriendship, geometricMeanOfRivalry);
	if (result==NULL){
		#ifndef DNDEBUG
		printf("IsraeliQueueMerge: IsraeliQueueCreate failed");
		#endif
		return result;
	}
	int temp=sizeOfNewQueue-1;
	void *currentObject;
	while (temp>0){
		for (int i=0; i<count; i++){
			if (queueArray[i]->m_size>1){
				currentObject=IsraeliQueueDequeue(queueArray[i]);
				if (currentObject==NULL){
					#ifndef DNDEBUG
					printf("IsraeliQueueMerge: IsraeliQueueDequeue failed");
					#endif
					return NULL;
				}
				if(IsraeliQueueEnqueue(result, currentObject)!=ISRAELIQUEUE_SUCCESS){
					#ifndef DNDEBUG
					printf("IsraeliQueueMerge: IsraeliQueueEnqueue failed");
					#endif
					return NULL;
				}
				temp--;
			}
				
		}
	}
	result->m_objects[sizeOfNewQueue-1]=NULL;
	return result;
} 
//test 4



int comparison_function_mock(void *obj1, void *obj2) {
    int id1 = *(int *)obj1;
    int id2 = *(int *)obj2;

    return id1 - id2;
}
int mockfriendshipfunction(void* firstObject, void* secondObject){
	int temp = (*(int*)firstObject)+(*(int*)firstObject)+5;
	return temp;
}

void PrintIsraeliQueue(IsraeliQueue queue) {
    if (queue == NULL) {
        printf("Invalid queue.\n");
        return;
    }

    printf("IsraeliQueue content:\n");
    for (int i = 0; i < queue->m_size - 1; i++) {
        printf("%d ", *((int *)queue->m_objects[i]));
    }
    printf("\n");
}


int main(){
	int arr[]={1,2,3,4};
	FriendshipFunction functions[]={mockfriendshipfunction, NULL};
	IsraeliQueue queue=IsraeliQueueCreate(functions, comparison_function_mock, 0, 0);
	for (int i=0; i<4; i++){
		IsraeliQueueEnqueue(queue, &arr[i]);
		
	}
	bool test=false;
	printf("%d\n", test);
	IsraeliQueueAddFriendshipMeasure(queue, mockfriendshipfunction);
	IsraeliQueue p=IsraeliQueueClone(queue);
	IsraeliQueue j=IsraeliQueueClone(queue);
	IsraeliQueue m=IsraeliQueueClone(queue);
	IsraeliQueue s=IsraeliQueueClone(queue);
	IsraeliQueueDequeue(p);
	IsraeliQueueDequeue(p);
	IsraeliQueueDequeue(j);
	IsraeliQueueDequeue(j);
	IsraeliQueueDequeue(j);
	IsraeliQueueDequeue(m);

	IsraeliQueueImprovePositions(queue);
	
	IsraeliQueue f[]={queue, p, j, m, s , NULL};
	IsraeliQueue g=IsraeliQueueMerge(f, comparison_function_mock);
	PrintIsraeliQueue(queue);
	PrintIsraeliQueue(p);
	IsraeliQueueDestroy(queue);
	PrintIsraeliQueue(g);
	

	return 0;
}

//test 3
/*
int arr[]={7,6,5,4,3,2,1};

int MockFriendshipFunction(void* firstObject, void* secondObject){// 8 and 5
	if ((*(int*)firstObject==7 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==7)){
		return 9;
	}

	if ((*(int*)firstObject==6 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==6)){
		return 4;
	}
	if ((*(int*)firstObject==5 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==5)){
		return 4;
	}

	if ((*(int*)firstObject==4 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==4)){
		return 9;
	}
	if ((*(int*)firstObject==3 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==3)){
		return 4;
	}

	if ((*(int*)firstObject==2 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==2)){
		return 9;
	}
	if ((*(int*)firstObject==1 && *(int*)secondObject==8) || (*(int*)firstObject==8 && *(int*)secondObject==1)){
		return 9;
	}

		return 6; //6 is neutral 4 is rival and 9 is friend


}

int comparison_function_mock(void *obj1, void *obj2) {
    int id1 = *(int *)obj1;
    int id2 = *(int *)obj2;

    return id1 - id2;
}

FriendshipFunction friendshipFunctions[]={NULL};

int eight_value = 8;
int *eight = &eight_value;

void PrintIsraeliQueue(IsraeliQueue queue) {
    if (queue == NULL) {
        printf("Invalid queue.\n");
        return;
    }

    printf("IsraeliQueue content:\n");
    for (int i = 0; i < queue->m_size - 1; i++) {
        printf("%d ", *((int *)queue->m_objects[i]));
    }
    printf("\n");
}


void Test3(){
	


	IsraeliQueue queue=IsraeliQueueCreate(friendshipFunctions, comparison_function_mock, 8, 5);

	IsraeliQueueAddFriendshipMeasure(queue,MockFriendshipFunction);
	IsraeliQueueAddFriendshipMeasure(queue, MockFriendshipFunction);
	
	for (int i=0; i<7; i++){
		IsraeliQueueEnqueue(queue, &arr[i]);
	}
	IsraeliQueueEnqueue(queue, eight);
	IsraeliQueue p=IsraeliQueueClone(queue);

	int* out=(int*)IsraeliQueueDequeue(queue);
	PrintIsraeliQueue(p);
	PrintIsraeliQueue(queue);
	printf("out is %d\n", *out);
	for (int i=0; i<7; i++){
		if (IsraeliQueueContains(p, &arr[i])!=true){
		printf("IsraeliQueueContains failed on p, doesn't contain %d\n", arr[i]);	
		}
	}
	if (IsraeliQueueContains(p, eight)!=true){
		printf("IsraeliQueueContains failed on p, doesn't contain 8\n");	
	}

	for (int i=1; i<7; i++){
		if (IsraeliQueueContains(queue, &arr[i])!=true){
		printf("IsraeliQueueContains failed on queue, doesn't contain %d\n", arr[i]);
		}	
	}
	if(IsraeliQueueContains(queue, eight)!=true){
		printf("IsraeliQueueContains failed on queue, doesn't contains 8\n");	
	}

	IsraeliQueueAddFriendshipMeasure(queue, MockFriendshipFunction);
	IsraeliQueueAddFriendshipMeasure(queue, MockFriendshipFunction);
	IsraeliQueue queues_arr[]={queue, p, NULL};
	FriendshipFunction* functions=getAllFriendshipFunctions(queues_arr);
	printf("functions[0] is %d\n", functions[0](eight, &arr[0]));
	IsraeliQueue f=IsraeliQueueClone(p);
	IsraeliQueue g=IsraeliQueueMerge(queues_arr, comparison_function_mock);
	PrintIsraeliQueue(g);
	IsraeliQueueImprovePositions(g);
	PrintIsraeliQueue(g);
	PrintIsraeliQueue(f);
	IsraeliQueueImprovePositions(f);
	PrintIsraeliQueue(f);

}

int main(){
	Test3();
	return 0;
}


//test 2


typedef struct {
    int age;
    char* name;
    char* hobbies[5];
} Person;

// A friendship function based on age difference
int AgeDifferenceFriendship(void* firstObject, void* secondObject) {
    Person* person1 = (Person*)firstObject;
    Person* person2 = (Person*)secondObject;

    int age_difference = abs(person1->age - person2->age);
    return 100 - age_difference * 2;
}

// A friendship function based on shared hobbies
int SharedHobbiesFriendship(void* firstObject, void* secondObject) {
    Person* person1 = (Person*)firstObject;
    Person* person2 = (Person*)secondObject;

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

    int friendshipThresholdreshold = 85;
    int rivalryThresholdreshold = 60;

    Relation result = checkRelation(friendship_functions, &alice, &bob, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 1: Alice and Bob - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));


    result = checkRelation(friendship_functions, &alice, &charlie, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 2: Alice and Charlie - Result: %s, Expected: RIVAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));


    result = checkRelation(friendship_functions, &alice, &dave, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 3: Alice and Dave - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = checkRelation(friendship_functions, &bob, &charlie, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 4: Bob and Charlie - Result: %s, Expected: NEUTRAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = checkRelation(friendship_functions, &bob, &dave, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 5: Bob and Dave - Result: %s, Expected: FRIEND\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));

    result = checkRelation(friendship_functions, &charlie, &dave, friendshipThresholdreshold, rivalryThresholdreshold);
    printf("Test 6: Charlie and Dave - Result: %s, Expected: RIVAL\n", result == FRIEND ? "FRIEND" : (result == RIVAL ? "RIVAL" : "NEUTRAL"));
}

int main() {
    Test_CheckRelation();
    return 0;
}

	
 //test 1

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
    IsraeliQueue queue = IsraeliQueueCreate(friendship_functions, comparison_function_mock, 2, 3);

    if (!queue) {
        printf("Error: Failed to create IsraeliQueue\n");
        return 1;
    }

    // Create and enqueue sample objects (integers in this case)
    for (int i = 0; i < 5; i++) {
        int *new_object = malloc(sizeof(int));
        *new_object = i;
        IsraeliQueueError err = IsraeliQueueEnqueue(queue, new_object);

        if (err != ISRAELIQUEUE_SUCCESS) {
            printf("Error: Failed to enqueue object %d\n", i);
            return 1;
        }
    }

    // Display the queue
    printf("Queue after enqueuing objects:\n");
    for (int i = 0; i < queue->m_size; i++) {
        int *obj = (int *)queue->m_objects[i];
        printf("Object ID: %d, Friendship Quota: %d, Rivalry Quota: %d\n", *obj, queue->m_quotas[i].m_friendshipQuota, queue->m_quotas[i].m_rivalryQuota);
    }

    // Clone the queue
    IsraeliQueue cloned_q = IsraeliQueueClone(queue);

    if (!cloned_q) {
        printf("Error: Failed to clone IsraeliQueue\n");
        return 1;
    }

    // Display the cloned queue
    printf("\nCloned queue:\n");
    for (int i = 0; i < cloned_q->m_size; i++) {
        int *obj = (int *)cloned_q->m_objects[i];
        printf("Object ID: %d, Friendship Quota: %d, Rivalry Quota: %d\n", *obj, cloned_q->m_quotas[i].m_friendshipQuota, cloned_q->m_quotas[i].m_rivalryQuota);
    }

    // Cleanup
    for (int i = 0; i < queue->m_size; i++) {
        free(queue->m_objects[i]);
    }

    IsraeliQueueDestroy(queue);
    IsraeliQueueDestroy(cloned_q);

    return 0;
} */