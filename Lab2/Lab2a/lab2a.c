
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "fcntl.h"
#include <pthread.h>

long long counter;



void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
    *pointer = sum;
}


void *thread_func(void* arg) {
	int iter =  *((int*) arg);
	int i;
	
	for (i=0;i<iter;i++) {
		add(&counter,(long long)1);
	}

	for (i=0;i<iter;i++) {
		add(&counter,(long long)-1);
	}
}

int main(int argc, char* argv[]) {
	counter = 0;
	int numThreads;
	int numIterations;
	struct option long_options[] = {
		{"threads",required_argument,0,'t'},
		{"iterations",required_argument,0,'i'},
		{NULL,0,NULL,0}
	};
	int index = 0;
	int c = 0;
	while (c != -1){
		opterr= 0;
		c = getopt_long(argc,argv,"",long_options,&index);
		switch (c) {
			case 't':
				numThreads = atoi(optarg);
				fprintf(stdout, "%d\n", numThreads);
				break;
			case 'i':
				numIterations = atoi(optarg);
				fprintf(stdout, "%d\n", numIterations);
				break;
			case '?':
				fprintf(stderr, "Invalid Option!\n" );
				break;
			
		}
	}
	//Got options
	pthread_t* thread_array = malloc(numThreads*sizeof(pthread_t));

	//counter =100;
	long long x = 1;
	int i;
	for (i = 0;i<numThreads;i++) {
		
		pthread_create(&thread_array[i], NULL,thread_func,&numIterations);
	}
	for (i=0;i<numThreads;i++) {
		pthread_join(thread_array[i],NULL);
	}
	
	printf("%lld\n", counter);
}