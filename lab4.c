#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define THREAD_CREATE_SUCCESS 0
#define PTHREAD_CANCEL_SUCCESS 0
#define STRINGS_AMOUNT 10000000
#define WAIT_TIME 2

void *thread_func(void *parameters){
	char text[] = "printing text...\n";
	size_t textLength = strlen(text);	
	for (;;){
		write(0, text, textLength);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv){
	pthread_t threadID;
	int threadCreateReturn, i, pthreadCancelReturn;
	threadCreateReturn = pthread_create(&threadID, NULL, thread_func, NULL);
	if (threadCreateReturn != THREAD_CREATE_SUCCESS){
		perror("Error in creating new thread");
		return EXIT_FAILURE;
	}
	sleep(WAIT_TIME);
	pthreadCancelReturn = pthread_cancel(threadID);
	if(pthreadCancelReturn != PTHREAD_CANCEL_SUCCESS){
		perror("Error in pthread_cancel");
		return EXIT_FAILURE;
	}
	printf("Child thread was canceled\n");
	pthread_exit(NULL);
}
