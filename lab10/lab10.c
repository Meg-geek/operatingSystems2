#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define THREAD_CREATE_SUCCESS 0
#define STRINGS_AMOUNT 10
#define MUTEX_AMOUNT 3
#define MUTEXATTR_INIT_SUCCESS 0
#define MUTEXATTR_DESTROY_SUCCESS 0
#define MUTEXATTR_SETTYPE_SUCCESS 0
#define MUTEX_INIT_SUCCESS 0
#define MUTEX_LOCK_SUCCESS 0
#define MUTEX_UNLOCK_SUCCESS 0
#define MUTEX_DESTROY_SUCCESS 0
#define PTHREAD_JOIN_SUCCESS 0

#define PRINT_MUTEX 0
#define FIRST 1
#define SECOND 2

#define SLEEP_TIME 1


pthread_mutex_t mutexArray[MUTEX_AMOUNT];
pthread_mutexattr_t mutexAttr;


void freeMemory();

void lockMutex(int mutexNumber){
	if(mutexNumber >= MUTEX_AMOUNT){
		return;
	}
	int mutexLockResult = pthread_mutex_lock(&mutexArray[mutexNumber]);
	if(mutexLockResult != MUTEX_LOCK_SUCCESS){
		perror("mutex_lock error");
	}
}

void unlockMutex(int mutexNumber){
	if(mutexNumber >= MUTEX_AMOUNT){
		return;
	}
	int mutexLockResult = pthread_mutex_unlock(&mutexArray[mutexNumber]);
	if(mutexLockResult != MUTEX_UNLOCK_SUCCESS){
		perror("mutex_unlock error");
	}
}

void *printSecond(void *parameters){
	lockMutex(SECOND);
	int i;
	for (i = 0; i < STRINGS_AMOUNT; i++){
		lockMutex(PRINT_MUTEX);
		printf("child thread, №%d\n", i);
		unlockMutex(SECOND);
		lockMutex(FIRST);
		unlockMutex(PRINT_MUTEX);
		lockMutex(SECOND);
		unlockMutex(FIRST);
	}
	unlockMutex(SECOND);
	return NULL;
}

void printFirst(){
	int i;
	for (i = 0; i < STRINGS_AMOUNT; i++){
		printf("parent thread, №%d\n", i);
		lockMutex(FIRST);
		unlockMutex(PRINT_MUTEX);
		lockMutex(SECOND);
		unlockMutex(FIRST);
		lockMutex(PRINT_MUTEX);
		unlockMutex(SECOND);
	}
	unlockMutex(PRINT_MUTEX);
}

void destroyMutexes(int amount){
	int i, destroyResult;
	for(i = 0; i < amount; i++){
		destroyResult = pthread_mutex_destroy(&mutexArray[i]);
		if(destroyResult != MUTEX_DESTROY_SUCCESS){
			perror("pthread_mutex_destroy error");
		}
	}
}

int initMutexes(){
	int funcResult, i;
	funcResult = pthread_mutexattr_init(&mutexAttr);
	if(funcResult != MUTEXATTR_INIT_SUCCESS){
		perror("mutexattr_init error");
		return EXIT_FAILURE;
	}
	funcResult = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
	if(funcResult != MUTEXATTR_SETTYPE_SUCCESS){
		perror("mutexattr_settype error");
		return EXIT_FAILURE;
	}
	for(i = 0; i < MUTEX_AMOUNT; i++){
		funcResult = pthread_mutex_init(&mutexArray[i], &mutexAttr);
		if(funcResult != MUTEX_INIT_SUCCESS){
			destroyMutexes(i);
			perror("mutex_init error");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv){
	pthread_t threadID;
	int threadCreateReturn, i, funcReturn;
	
	funcReturn = initMutexes();
	lockMutex(PRINT_MUTEX);
	
	threadCreateReturn = pthread_create(&threadID, NULL, printSecond, NULL);
	
	if (threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("Error in creating new thread");
		destroyMutexes(MUTEX_AMOUNT);
		return EXIT_FAILURE;
	}
	//for initialization
	sleep(SLEEP_TIME);
	
	printFirst();
	funcReturn = pthread_join(threadID, NULL);
	if(funcReturn != PTHREAD_JOIN_SUCCESS){
		perror("pthread_join error");
	}
	destroyMutexes(MUTEX_AMOUNT);
	pthread_exit(NULL);
}
