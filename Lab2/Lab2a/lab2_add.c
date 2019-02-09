
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
int opt_yield;


void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
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
	return NULL;
}

int main(int argc, char* argv[]) {
	counter = 0;
	opt_yield = 0;
	int numThreads;
	int numIterations;
	struct timespec times;
	long long starttime;
	long long endtime;
	clock_gettime(CLOCK_MONOTONIC,&times);
	starttime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;
	//endtime = (double) times.tv_sec + (double)  times.tv_nsec * 0.000000001;
	//fprintf(stdout, "%f\n", starttime);

	struct option long_options[] = {
		{"yield", no_argument,&opt_yield,1},
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
				//fprintf(stdout, "%d Threads\n", numThreads);
				break;
			case 'i':
				numIterations = atoi(optarg);
				//fprintf(stdout, "%d Iterations\n", numIterations);
				break;
			case '?':
				fprintf(stderr, "Invalid Option!\n" );
				break;
			
		}
	}
	//Got options
	pthread_t* thread_array = malloc(numThreads*sizeof(pthread_t));

	//counter =100;
	//long long x = 1;
	int i;
	for (i = 0;i<numThreads;i++) {
		
		pthread_create(&thread_array[i], NULL,thread_func,&numIterations);
	}
	for (i=0;i<numThreads;i++) {
		pthread_join(thread_array[i],NULL);
	}
	if (!opt_yield) {
		printf("add-none,");
	}
	else
	{
		printf("add-yield,");
	}

	
	clock_gettime(CLOCK_MONOTONIC,&times);
	endtime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;
	printf("%d,",numThreads);
	printf("%d,", numIterations);
	printf("%d,", numThreads*numIterations*2);
	printf("%lld,", endtime - starttime);
	printf("%lld\n", counter);
	//fprintf(stdout, "Total time: %lld\n", endtime - starttime);
}