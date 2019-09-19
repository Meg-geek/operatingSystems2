#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#define WRONG_INPUT -1
#define MAX_ITERATIONS_AMOUNT 10000000
#define ITERATION_CHECK 1000
#define ITERATIONS_DELAY 10000
#define THREAD_CREATE_SUCCESS 0
#define PTHREAD_JOIN_SUCCESS 0
#define TRUE 1
#define FALSE 0
#define SIGSET_ERROR -1
#define SIGADDSET_ERROR -1

struct ThreadInfo {
	int threadNumb;
	double threadPi;
	int threadsAmount;
};

int sigintCaugth = FALSE;

void *countPi(void *parameters){
	int threadNumb = ((struct ThreadInfo*)parameters)->threadNumb;
	int threadsAmount = ((struct ThreadInfo*)parameters)->threadsAmount;
	long long i = threadNumb, checkIterations;
	double localPi = 0;
	while(sigintCaugth == FALSE && i < MAX_ITERATIONS_AMOUNT){
		checkIterations = i + ITERATION_CHECK;
		for(i; i < checkIterations; i+= threadsAmount){
			localPi += 1.0/(i*4.0 + 1.0);
			localPi -= 1.0/(i*4.0 + 3.0);
		}
		
	}
	if(sigintCaugth == TRUE){
		checkIterations = i + ITERATIONS_DELAY;
		for(; i < checkIterations; i += threadsAmount){
			localPi += 1.0/(i*4.0 + 1.0);
			localPi -= 1.0/(i*4.0 + 3.0);
		}	
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
		(threadsInfo[i]).threadNumb = i;
		(threadsInfo[i]).threadsAmount = threadsAmount;
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

int addSignalToThread(int signo){
	sigset_t *oldset;
	int funcReturn = pthread_sigmask(FALSE, NULL, oldset);
	if(funcReturn != 0){
		perror("pthread_sigmask error");
		return EXIT_FAILURE;
	}
	funcReturn = sigaddset(oldset, SIG_IGN);
	funcReturn = pthread_sigmask(SIG_SETMASK, oldset, NULL);
	if(funcReturn != 0){
		perror("pthread_sigmask error");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
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
	int sigsetReturn = sigset(SIGINT, SIG_IGN);
	if(sigsetReturn == SIGSET_ERROR){
		perror("sigset error ");
	}
	int createThreadsResult = createThreads(threadIDs, threadsInfo, threadsAmount);
	if(createThreadsResult == EXIT_FAILURE){
		freeMemory(threadIDs, threadsInfo);
		return EXIT_FAILURE;
	}
	int sigintAddReturn = addSignalToThread(SIGINT);
	printf("\nsigintSetReturn %d \n", sigintAddReturn);
	double pi = getPi(threadIDs, threadsAmount);
	if (pi == EXIT_FAILURE){
		freeMemory(threadIDs, threadsInfo);
		return EXIT_FAILURE;
	}
	printf("\nPI was counted by %d threads\nResult: %lf\n", threadsAmount, pi);
	freeMemory(threadIDs, threadsInfo);
	return EXIT_SUCCESS;
}
