#include <stdlib.h>
#include <stdio.h>
#include <IsraeliQueue.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define UNINITIALIZED -1
#define CANNOT_FIND -2
#define ENQUEUE_FAILED -3

typedef enum { FRIEND, RIVAL, NEUTRAL } Relation;

typedef struct{ //we implemented using an array, not a linked list and we use the quota struct to keep track
	int m_friendshipQuota; // of the friendship and rivalry quotas and the improve field is to so we don't improve
	int m_rivalryQuota; // the same item twice in the IsraeliQueueImprovePosition function
	void* m_object;
	bool improved;
} Quota;

//Helper function that copies the friendship functions from the original array so we never alter the original array
//And so we can freely add functions using AddFriendshipMeasure and free the memory when done
//without affecting userspace.
static FriendshipFunction* copyFriendshipFunctions(FriendshipFunction* friendshipFunctions, IsraeliQueueError* errorFlag){
	int size=0;
	while (friendshipFunctions[size]!=NULL){
		size++;
	}
	FriendshipFunction* newFriendshipFunctions = malloc((size+1)*sizeof(FriendshipFunction));
	if (newFriendshipFunctions==NULL){
	#ifndef NDEBUG
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
//Helper function to copy quotas when cloning the IsraeliQueue so that the cloned queue has the same memory.
static Quota* copyQuotas(Quota* quotas, int size, IsraeliQueueError* errorFlag){
	Quota* newQuotas = malloc(size*sizeof(Quota));
	if (newQuotas==NULL){
	#ifndef NDEBUG
	printf("copyQuotas: malloc failed in copyQuotas");
	#endif
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
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
	ComparisonFunction m_compare;
	void** m_objects;
	int m_size;
	Quota* m_quotas;
};

//Function which takes two objects and checks their relationship using the constraints defined in the assignment.
static Relation checkRelation(FriendshipFunction* friendshipFunctions,
 void* firstObject, void* secondObject, int friendshipThreshold, int rivalryThreshold ){
	
	Relation currentRelation=NEUTRAL;
	int sumOfFriendshipPoints=0;
	int count=0;
	//Iterates over the friendshipfunctions and checks if one of them is above the friendship threshold
	//If so it returns a FRIEND Relation immediately.
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
	// If the objects are not friends, we check if they are rivals.
	if (count!=0){
	averageFriendshipPoints=(double)sumOfFriendshipPoints/count;
	}
	if (averageFriendshipPoints<(double)rivalryThreshold){
		currentRelation=RIVAL;
		return currentRelation;
	}
	//If they are not rivals or friends they must be neutral. We assert that this is the case and return neutral.
	assert(currentRelation==NEUTRAL);
	return currentRelation;

	}

//Function for copying the items from one queue to another. Used in the clone function.
static void** copyObjectArray(void** objects, int size, IsraeliQueueError* errorFlag){
	void** newObjects = malloc(size*sizeof(void*));
	if (newObjects==NULL){
	#ifndef NDEBUG
	printf("copyObjectArray: malloc failed in copyObjectArray");
	#endif
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
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
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: friendshipFunctions or compare is NULL");
	#endif

		return NULL;
	}
	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue == NULL){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		return NULL;
	}
	//The errorFlag is used to keep track of errors while calling helper functions. Makes sure we don't leak memory.
	//And that we return the correct error code.
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;
	newQueue->m_friendshipThreshold = friendshipThreshold;
	newQueue->m_rivalryThreshold = rivalryThreshold;
	newQueue->m_friendshipFunctions = copyFriendshipFunctions(friendshipFunctions, &errorFlag);
	if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		free(newQueue);
		return NULL;
	}
	newQueue->m_compare = compare;
	newQueue->m_objects = malloc(sizeof(void *));
	if (newQueue->m_objects == NULL){
	#ifndef NDEBUG
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
	#ifndef NDEBUG
	printf("IsraeliQueueClone: queue is NULL");
	#endif

	return NULL;
	}
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;

	if (queue->m_size==1){
		IsraeliQueue newQueue=IsraeliQueueCreate(queue->m_friendshipFunctions, queue->m_compare,
		 queue->m_friendshipThreshold, queue->m_rivalryThreshold);
		 if (newQueue==NULL){
			#ifndef NDEBUG
			printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
			#endif
			return NULL;
		 }
		 return newQueue;
	}

	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		return NULL;
	}
	newQueue->m_friendshipThreshold=queue->m_friendshipThreshold;
	newQueue->m_rivalryThreshold=queue->m_rivalryThreshold;
	newQueue->m_friendshipFunctions=copyFriendshipFunctions(queue->m_friendshipFunctions, &errorFlag);
	newQueue->m_compare=queue->m_compare;
	newQueue->m_objects=copyObjectArray(queue->m_objects, queue->m_size, &errorFlag);
	newQueue->m_quotas=copyQuotas(queue->m_quotas, queue->m_size, &errorFlag);
	newQueue->m_size=queue->m_size;
	
	if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
	#ifndef NDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		free(newQueue);
		return NULL;
	}
	return newQueue;
}


void IsraeliQueueDestroy(IsraeliQueue queue){
	if (queue==NULL){
	#ifndef NDEBUG
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

	if (queue->m_friendshipFunctions!=NULL){
		free(queue->m_friendshipFunctions);
	}
		free(queue);
	return;
}
//Helper function which checks if two objects are friends and whether the friendship quota is full.
static bool isAvailableFriend(IsraeliQueue queue, void* secondObject, int i){
  return ((checkRelation(queue->m_friendshipFunctions, queue->m_objects[i],
			secondObject, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==FRIEND)
		 	&& (queue->m_quotas[i].m_friendshipQuota<FRIEND_QUOTA));

}

static bool isAvailableRival(IsraeliQueue queue, void* secondObject, int j){
  return ((checkRelation(queue->m_friendshipFunctions, queue->m_objects[j],
			secondObject, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==RIVAL)
		 	&& (queue->m_quotas[j].m_rivalryQuota<RIVAL_QUOTA));

}

//Helper for IsraeliQueueEnqueue, which checks where we should enqueue the given object.
static void checkInsertLocation(IsraeliQueue queue, void* object, int *friendLocation, bool *skippedToFriend){

	bool blocked=false; //Initialize to no block. We update this if a block occured.
    for (int i=0; i<((queue->m_size)-1); i++){ //iterates from beginning to end searching for a friend
        *friendLocation=UNINITIALIZED;
        *skippedToFriend=false, blocked=false;
        if(isAvailableFriend(queue, object, i)){ //A friend is found at index i which didn't exceed quota
            for(int j=i+1; j<((queue->m_size)-1);  j++){ // Checks if there is a rival behind the friend
                if (isAvailableRival(queue, object, j)){ //A rival is found at index j which didn't exceed quota
                    queue->m_quotas[j].m_rivalryQuota++;
                    i=j; //Changes i to j so the search for next friend is behind the blocking rival.
                    blocked=true;
                    break;
                }
            }
            if (!(blocked)){ // If a friend is found and no block occured, insert object behind friend and quit
                *friendLocation=i; 
                *skippedToFriend=true;
                queue->m_quotas[*friendLocation].m_friendshipQuota++;
                break;
            }
        }
    }
}


//Helper function for IsraeliQueueEnqueue, after we found the location to insert the object, we insert it.
static void insertObject(IsraeliQueue queue, void* object, int* friendLocation, bool* skippedToFriend, bool improving){
	//If we skipped to a friend, we need to move all objects after the friend one index to the right.
	//After that we insert the object behind the friend.
	if (*skippedToFriend){ 
		for (int i=queue->m_size-1; i>*friendLocation; i--){
			queue->m_objects[i]=queue->m_objects[i-1];
			queue->m_quotas[i]=queue->m_quotas[i-1];
		}
		queue->m_objects[*friendLocation+1]=object;
		queue->m_quotas[*friendLocation+1].m_friendshipQuota=0;
		queue->m_quotas[*friendLocation+1].m_rivalryQuota=0;
		queue->m_quotas[*friendLocation+1].m_object=object;
		//the improving flag is to keep track of whether the insertion happened during ImprovePositions.
		//If we're inserting during IsraeliQueueImprovePosition we need to make sure that we don't dequeue and enqueue
		//The same object twice.
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

//Helper function for IsraeliQueueEnqueue, we resize the queue before we insert a new object.
static IsraeliQueueError resizeQueue(IsraeliQueue queue){
	void *tempPtr=queue->m_objects;
	queue->m_objects=realloc(queue->m_objects, (queue->m_size+1)*sizeof(void*));
	if (queue->m_objects==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		free (tempPtr);
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	queue->m_size+=1;
	queue->m_objects[queue->m_size-1]=NULL;
	tempPtr=queue->m_quotas;
	queue->m_quotas=realloc(queue->m_quotas, (queue->m_size+1)*sizeof(Quota));
	if (queue->m_quotas==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		free (tempPtr);
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	return ISRAELIQUEUE_SUCCESS;
}

// Helper function that enqueues an object and returns its index. Used during ImprovePositions.
//So we know where we enqueued an object so we can update its improved flag and restore its quota after we dequeue it.
static int enqueueAndGetIndex(IsraeliQueue queue, void* object, IsraeliQueueError* errorFlag){
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){	
		return ENQUEUE_FAILED;
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
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
	if (queue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: queue or object is NULL");
	#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	//Checks where we need to insert the object, updates friendLocation and skippedToFriend to inform insertObject.
	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){	
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	//Inserts the object in the queue. After a friend or at the end depending on the received values.
	insertObject(queue, object, &friendLocation, &skippedToFriend, false);
	return ISRAELIQUEUE_SUCCESS;
	}



void* IsraeliQueueDequeue(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: queue is NULL");
		#endif
		return NULL;
	}

	if (queue->m_size==1){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: queue is empty");
		#endif
		return NULL;
	}

	void* object=queue->m_objects[0];
	for (int i=0; i<queue->m_size-1; i++){
		queue->m_objects[i]=queue->m_objects[i+1];
		queue->m_quotas[i]=queue->m_quotas[i+1];
	}
	void *tempPtr=queue->m_objects;
	queue->m_objects=realloc(queue->m_objects, (queue->m_size-1)*sizeof(void*));
	if (queue->m_objects==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: realloc failed in IsraeliQueueDequeue");
		#endif
		free(tempPtr);
		return NULL;
	}
	queue->m_size-=1;
	return object;

}


bool IsraeliQueueContains(IsraeliQueue queue, void * object){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueContains: queue is NULL");
		#endif
		return false;
	}

	if (object==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueContains: object is NULL");
		#endif
		return false;
	}
	//Iterates over all the items and compares them to the received object using the compare function.
	for (int i=0; i<queue->m_size-1; i++){
		if (queue->m_compare(queue->m_objects[i], object)){
			return true;
		}
	}
	return false;


}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction newFriendshipFunction) {
    if (queue == NULL || newFriendshipFunction == NULL) {
        #ifndef NDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: queue or newFriendshipFunction is NULL");
        #endif
        return ISRAELIQUEUE_BAD_PARAM;
    }

    int count = 0;
    while (queue->m_friendshipFunctions[count] != NULL) {
        count++;
    }
	void *tempPtr=queue->m_friendshipFunctions;
    queue->m_friendshipFunctions = realloc(queue->m_friendshipFunctions,(count + 2) * sizeof(FriendshipFunction));
    if (queue->m_friendshipFunctions == NULL) {
        #ifndef NDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: malloc failed");
        #endif
		free(tempPtr);
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    queue->m_friendshipFunctions[count] = newFriendshipFunction;
    queue->m_friendshipFunctions[count + 1] = NULL;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueUpdateFriendshipThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_friendshipThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueUpdateRivalryThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_rivalryThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;

}

int IsraeliQueueSize(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueSize: queue is NULL");
		#endif
		return 0;
	}

	return queue->m_size-1;
}
//Helper function that dequeues an item at a specific index. Used during ImproveQueue
//because we need to remove from the middle many times.
static void* IsraeliQueueDequeueAtIndex(IsraeliQueue queue, int index, IsraeliQueueError* errorFlag){
	if (queue == NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: queue is NULL");
		#endif
		return NULL;
	}
	if (index < 0 || index >= queue->m_size - 1){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: invalid index");
		#endif
		return NULL;
	}
	void *tempPtr=queue->m_objects;
	void** objects = realloc(queue->m_objects, (queue->m_size - 1) * sizeof(void*));
	if (objects == NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: realloc failed in IsraeliQueueDequeueAtIndex");
		#endif
		free(tempPtr);
		*errorFlag = ISRAELIQUEUE_ALLOC_FAILED;
		return NULL;
	}
	void* object = queue->m_objects[index];
	for (int i = index; i < queue->m_size - 1; i++){
		queue->m_objects[i] = queue->m_objects[i + 1];
		queue->m_quotas[i] = queue->m_quotas[i + 1];
	}
	queue->m_objects = objects;
	queue->m_size -= 1;
	return object;
}

//Copies the queue and the goes from end to start dequeuing and enqueuing according to the original order
//Uses improved bool flag to keep track of items that were already improved.
//This way we don't dequeue and enqueue the same item twice.
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueImprovePosition: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	if (queue->m_size<=1){
		return ISRAELIQUEUE_SUCCESS;
	}
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;//To keep track of errors during helper functions
	IsraeliQueue originalQueue=IsraeliQueueClone(queue);
	if (originalQueue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueImprovePosition: clone failed");
		#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	for (int i=originalQueue->m_size-2; i>=0; i--){
		void* currentObject=originalQueue->m_objects[i];
		for (int j=originalQueue->m_size-2; j>=0; j--){//iterate from end to beginning dequeuing and enqueueing items
			if ((currentObject==(queue->m_objects[j])) && !(queue->m_quotas[j].improved)){
				Quota currentQuota=queue->m_quotas[j];
				IsraeliQueueDequeueAtIndex(queue, j, &errorFlag);
				if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){//boilerplate error handling
					#ifndef NDEBUG
					printf("IsraeliQueueImprovePosition: dequeue failed");
					#endif
					IsraeliQueueDestroy(originalQueue);
					return ISRAELIQUEUE_ALLOC_FAILED;
				}
				int index=enqueueAndGetIndex(queue, currentObject, &errorFlag);
				if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){//boilerplate error handling
					#ifndef NDEBUG
					printf("IsraeliQueueImprovePosition: enqueue failed");
					#endif
					IsraeliQueueDestroy(originalQueue);
					return ISRAELIQUEUE_ALLOC_FAILED;
				}
				queue->m_quotas[index]=currentQuota; //retain quota
				queue->m_quotas[index].improved=true;
				break;
			}
		}
	}
	for (int i=0; i<queue->m_size-1; i++){	//reset improved flags
		queue->m_quotas[i].improved=false;
	}
	IsraeliQueueDestroy(originalQueue);
	return ISRAELIQUEUE_SUCCESS;
}

//Helper function used during merge. Takes an array of queues and returns a new array
//that contains all of the friendship functions.
static FriendshipFunction* getAllFriendshipFunctions(IsraeliQueue* queues) {
    int totalSize = 0;

    for (IsraeliQueue* queuePointer=queues; *queuePointer != NULL; queuePointer++) {
        for (FriendshipFunction* functionsPointer = (*queuePointer)->m_friendshipFunctions;
		 *functionsPointer != NULL; functionsPointer++) {
            
			totalSize++;
        }
    }

    FriendshipFunction* result = (FriendshipFunction*) malloc((totalSize + 1) * sizeof(FriendshipFunction));
    if (result == NULL) {
		#ifndef NDEBUG
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
	if (queueArray==NULL || compare==NULL || (*queueArray==NULL)){
		#ifndef NDEBUG
		printf("IsraeliQueueMerge: queueArray or compare is NULL");
		#endif
		return NULL;
	}//calculate size of resulting queue and prepare to calculate averages
	int count=0, sizeOfNewQueue=0, sumOfFriendshipThresholds=0, productOfRivalryThresholds=1;
	while (queueArray[count]!=NULL){
		sizeOfNewQueue+=(queueArray[count]->m_size-1);
		sumOfFriendshipThresholds+=queueArray[count]->m_friendshipThreshold;
		productOfRivalryThresholds*=queueArray[count]->m_rivalryThreshold;
		count++;
	}
	sizeOfNewQueue+=1;// preparing to terminate with NULL this is due to how we implemented queues.
	FriendshipFunction* friendshipFunctions=getAllFriendshipFunctions(queueArray);//get all friendship functions
	int averageFriendship=(int)ceil((double)sumOfFriendshipThresholds/count);
	int geometricMeanOfRivalry = (int)ceil(pow((double)abs(productOfRivalryThresholds), 1.0 / count));
	IsraeliQueue result=IsraeliQueueCreate(friendshipFunctions, compare, averageFriendship, geometricMeanOfRivalry);
	if (result==NULL){
		#ifndef NDEBUG
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
				if(IsraeliQueueEnqueue(result, currentObject)!=ISRAELIQUEUE_SUCCESS){
					#ifndef NDEBUG
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
