//compile with -lrt
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define THREAD_CREATE_SUCCESS 0
#define STRINGS_AMOUNT 10
#define SEM_INIT_ERROR -1
#define SEM_DESTROY_SUCCESS 0
#define CHILD_SEM_INIT 0
#define PARENT_SEM_INIT 1

sem_t childSem, parentSem;

void freeMemory();

void *thread_body(void *parameters){
	int i;
	for (i = 0; i < STRINGS_AMOUNT; i++){
		sem_wait(&childSem);
		printf("child thread, №%d\n", i);
		sem_post(&parentSem);
	}
	//freeMemory();
	return NULL;
}

void freeMemory(){
	int destroyResult = sem_destroy(&childSem);
	if(destroyResult !=SEM_DESTROY_SUCCESS){
		perror("sem_destroy error");
	}
	destroyResult = sem_destroy(&parentSem);
	if(destroyResult !=SEM_DESTROY_SUCCESS){
		perror("sem_destroy error");
	}
}

int initSemaphores(){
	int funcReturn;
	funcReturn = sem_init(&childSem, 0, CHILD_SEM_INIT);
	if(funcReturn == SEM_INIT_ERROR){
		return EXIT_FAILURE;
	}
	funcReturn = sem_init(&parentSem, 0, PARENT_SEM_INIT);
	if(funcReturn == SEM_INIT_ERROR){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv){
	pthread_t threadID;
	int threadCreateReturn, i, funcReturn;
	funcReturn = initSemaphores();
	if(funcReturn == EXIT_FAILURE){
		perror("sem_init error");
		freeMemory();
		return EXIT_FAILURE;
	}
	
	threadCreateReturn = pthread_create(&threadID, NULL, thread_body, NULL);
	if (threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("Error in creating new thread");
		freeMemory();
		return EXIT_FAILURE;
	}
	
	for (i = 0; i < STRINGS_AMOUNT; i++){
		sem_wait(&parentSem);
		printf("parent thread, №%d\n", i);
		sem_post(&childSem);
	}
	pthread_join(threadID, NULL);
	freeMemory();
	pthread_exit(NULL);
}
