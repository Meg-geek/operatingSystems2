#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>

#define WRONG_INPUT -1
#define ITERATIONS_AMOUNT 10000
#define THREAD_CREATE_SUCCESS 0
#define PTHREAD_JOIN_SUCCESS 0

struct ThreadInfo {
	int threadNumb;
	double threadPi;
};


void *countPi(void *parameters){
	int threadNumb = ((struct ThreadInfo*)parameters)->threadNumb;
	int i = ITERATIONS_AMOUNT * threadNumb;
	double localPi = 0;
	for(i; i < ITERATIONS_AMOUNT*(threadNumb + 1); i++){
		localPi += 1.0/(i*4.0 + 1.0);
        localPi -= 1.0/(i*4.0 + 3.0);
	}
	((struct ThreadInfo*)parameters)->threadPi = localPi;
	pthread_exit(parameters);
}

int getNumb(char *line){
	errno = 0;
	char *end;
	long returnStrtol = strtol(line, &end, 10);
	// nothing parsed from the string, handle errors or exit
	if (end == line){
		return WRONG_INPUT;
	}
	if ((returnStrtol == LONG_MAX || returnStrtol == LONG_MIN) && errno == ERANGE)
	{
   	 // out of range, handle or exit
		return WRONG_INPUT;
	}
	if(returnStrtol <= 0){ 
		return WRONG_INPUT;
	}
	return (int)returnStrtol;
}

int createThreads(pthread_t* threadsID, struct ThreadInfo* threadsInfo, int threadsAmount){
	int i, threadCreateReturn = THREAD_CREATE_SUCCESS;
	for(i = 0; i < threadsAmount && threadCreateReturn == THREAD_CREATE_SUCCESS; i++){
		(threadsInfo[i]).threadNumb = i; //or . ?
		threadCreateReturn = pthread_create((threadsID + i), NULL, countPi, (void*)(threadsInfo+i));
	}
	if(threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("error in pthread_create");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void freeMemory(pthread_t* threadsID, struct ThreadInfo* threadsInfo){
	free(threadsID);
	free(threadsInfo);
}

double getPi(pthread_t* threadsID, int threadsAmount){
	int i, threadJoinReturn = PTHREAD_JOIN_SUCCESS;
	double pi = 0;
	struct ThreadInfo *joinStatus;
	for(i = 0; i < threadsAmount && threadJoinReturn == PTHREAD_JOIN_SUCCESS; i++){
		threadJoinReturn = pthread_join(threadsID[i], (void**)&joinStatus); 
		pi+= joinStatus->threadPi;
	}
	if(threadJoinReturn != PTHREAD_JOIN_SUCCESS){
		perror("pthread_join error");
		return EXIT_FAILURE;
	}
	
	return pi*4;
}

int main(int argc, char **argv){
	if(argc < 2){
		fprintf(stderr, "Need threads amount\n");
		return EXIT_FAILURE;
	}
	int threadsAmount = getNumb(argv[1]);
	if(threadsAmount == WRONG_INPUT){
		fprintf(stderr, "Wrong imput parameter for threads amount, try again\n");
		return EXIT_FAILURE;
	}
	pthread_t *threadIDs;
	struct ThreadInfo *threadsInfo;
	threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * threadsAmount);
	threadsInfo = (struct ThreadInfo*)malloc(sizeof(struct ThreadInfo) * threadsAmount);
	if(threadIDs == NULL || threadsInfo == NULL){
		freeMemory(threadIDs, threadsInfo);
		perror("malloc error");
		return EXIT_FAILURE;
	}
	int createThreadsResult = createThreads(threadIDs, threadsInfo, threadsAmount);
	if(createThreadsResult == EXIT_FAILURE){
		freeMemory(threadIDs, threadsInfo);
		return EXIT_FAILURE;
	}
	double pi = getPi(threadIDs, threadsAmount);
	if (pi == EXIT_FAILURE){
		freeMemory(threadIDs, threadsInfo);
		return EXIT_FAILURE;
	}
	printf("PI was counted by %d threads\nResult: %lf\n", threadsAmount, pi);
	freeMemory(threadIDs, threadsInfo);
	return EXIT_SUCCESS;
}
