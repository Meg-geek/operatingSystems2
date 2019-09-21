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
#define SIGDELSET_ERROR -1
#define SIGWAIT_ERROR -1
#define PTHREAD_SIGMASK_SUCCESS 0
#define SLEEP_TIME 5

int delSigintInThread();

struct ThreadInfo {
	int threadNumb;
	double threadPi;
	int threadsAmount;
};

int sigintCaught = FALSE;

void *countPi(void *parameters){
	int threadNumb = ((struct ThreadInfo*)parameters)->threadNumb;
	int threadsAmount = ((struct ThreadInfo*)parameters)->threadsAmount;
	long long i = threadNumb, checkIterations;
	double localPi = 0;
	int delSign = delSigintInThread();
	if(delSign == EXIT_FAILURE){
		fprintf(stderr, "THREAD %d :SIGINT wasn't deleted from my signal mask", threadNumb);
	}
	while(sigintCaught == FALSE && i < MAX_ITERATIONS_AMOUNT){
		checkIterations = i + ITERATION_CHECK;
		for(i; i < checkIterations; i+= threadsAmount){
			localPi += 1.0/(i*4.0 + 1.0);
			localPi -= 1.0/(i*4.0 + 3.0);
		}
	}
	sleep(SLEEP_TIME);
	if(sigintCaught == TRUE){
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
	sleep(SLEEP_TIME);
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

int delSigintInThread(){
	sigset_t *oldset = (sigset_t*)malloc(sizeof(sigset_t));
	int funcReturn = pthread_sigmask(FALSE, NULL, oldset);
	if(funcReturn != PTHREAD_SIGMASK_SUCCESS){
		perror("pthread_sigmask error");
		free(oldset);
		return EXIT_FAILURE;
	}
	funcReturn = (int)sigdelset(oldset, SIGINT);
	if(funcReturn == SIGDELSET_ERROR){
		perror("sigdelset error");
		free(oldset);
		return EXIT_FAILURE;
	}
	funcReturn = pthread_sigmask(SIG_SETMASK, oldset, NULL);
	if(funcReturn != PTHREAD_SIGMASK_SUCCESS){
		perror("pthread_sigmask error");
		free(oldset);
		return EXIT_FAILURE;
	}
	free(oldset);
	return EXIT_SUCCESS;
}

void signalRepeatHandler(int sign){
	fprintf(stderr, "I'm interrupting...\n");
	void *signalReturn = signal(SIGINT, signalRepeatHandler);
	if(signalReturn == SIG_ERR){
		perror("signal error");
	}
}

void signalHandler(int sign){
	printf("Signal SIGINT was caught, start interrupting calculation..\n");
	sigintCaught = TRUE;
	void *signalReturn = signal(SIGINT, signalRepeatHandler);
	if(signalReturn == SIG_ERR){
		perror("signal error");
	}
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
	void *signalReturn = signal(SIGINT, signalHandler);
	if(signalReturn == SIG_ERR){
		perror("signal error");
		freeMemory(threadIDs, threadsInfo);
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
	printf("\nPI was counted by %d threads\nResult: %lf\n", threadsAmount, pi);
	freeMemory(threadIDs, threadsInfo);
	return EXIT_SUCCESS;
}
