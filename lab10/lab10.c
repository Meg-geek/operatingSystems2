#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define THREAD_CREATE_SUCCESS 0
#define STRINGS_AMOUNT 10
#define MUTEX_LOCK_SUCCESS 0
#define MUTEX_UNLOCK_SUCCESS 0
#define PARENT_WRITE 0
#define CHILD_WRITE 1
#define COND_DESTROY_SUCCESS 0
#define MUTEX_DESTROY_SUCCESS 0

pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writeCond = PTHREAD_COND_INITIALIZER;

int whoWrite = PARENT_WRITE;

void *thread_body(void *parameters){
	int i;
	/*pthread_mutex_lock(&writeMutex);
	while(whoWrite == PARENT_WRITE){
		pthread_cond_wait(&writeCond, &writeMutex);
	}*/
	for (i = 0; i < STRINGS_AMOUNT; i++){
		pthread_mutex_lock(&writeMutex);
		while(whoWrite == PARENT_WRITE){
			pthread_cond_wait(&writeCond, &writeMutex);
		}
		printf("child thread, №%d\n", i);
		whoWrite = PARENT_WRITE;
		pthread_cond_signal(&writeCond);
		pthread_mutex_unlock(&writeMutex);
	}
	return NULL;
}

void freeMemory(){
	int destroyResult = pthread_mutex_destroy(&writeMutex);
	if(destroyResult != MUTEX_DESTROY_SUCCESS){
		perror("pthread_mutex_destroy error");
	}
	destroyResult = pthread_cond_destroy(&writeCond);
	if(destroyResult != COND_DESTROY_SUCCESS){
		perror("pthread_mutex_destroy error");
	}
}

int main(int argc, char **argv){
	pthread_t threadID;
	int threadCreateReturn, i, funcReturn;
	threadCreateReturn = pthread_create(&threadID, NULL, thread_body, NULL);
	
	if (threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("Error in creating new thread");
		return EXIT_FAILURE;
	}
	
	for (i = 0; i < STRINGS_AMOUNT; i++){
		pthread_mutex_lock(&writeMutex);
		while(whoWrite == CHILD_WRITE){
			pthread_cond_wait(&writeCond, &writeMutex);
		}
		printf("parent thread, №%d\n", i);
		whoWrite = CHILD_WRITE;
		pthread_cond_signal(&writeCond);
		pthread_mutex_unlock(&writeMutex);
	}
	pthread_exit(NULL);
}
