#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define THREAD_CREATE_SUCCESS 0
#define STRINGS_AMOUNT 10
#define PTHREAD_JOIN_SUCCESS 0

void *thread_body(void *parameters){
	int i;
	for (i = 0; i < STRINGS_AMOUNT; i++){
		printf("child thread, №%d\n", i);
	}
	return NULL;
}

int main(int argc, char **argv){
	pthread_t thread;
	int threadCreateReturn, i, threadJoinReturn;
	threadCreateReturn = pthread_create(&thread, NULL, thread_body, NULL);
	
	if (threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("Error in creating new thread");
		return EXIT_FAILURE;
	}
	threadJoinReturn = pthread_join(thread, NULL);
	if (threadJoinReturn != PTHREAD_JOIN_SUCCESS){
		perror("Error in pthread_join");
	}
	
	
	for (i = 0; i < STRINGS_AMOUNT; i++){
		printf("parent thread, №%d\n", i);
	}
	pthread_exit(NULL);
}
