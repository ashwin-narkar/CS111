/*
NAME: Ashwin Narkar
ID: 204585474
EMAIL: ashwin.narkar@gmail.com
*/


#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include "fcntl.h"
#include <pthread.h>

//long long counter;
int opt_yield;


struct threadArgs {
	long long *c;
	int iterations;
	char* sync_opt;
};


void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
    *pointer = sum;
    
}

pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER;
void add_with_mutex(long long *pointer, long long value) {
	pthread_mutex_lock(&l);
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
    *pointer = sum;
    pthread_mutex_unlock(&l);
}

int spin_lock = 0;



void spin_lock_add(long long *pointer, long long value) {
	while (__sync_lock_test_and_set(&spin_lock,1));
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
    *pointer = sum;
    __sync_lock_release(&spin_lock);
}

void comp_and_swap_add(long long *pointer, long long value) {
	int t1,t2;
	do {
		t1 = *pointer;
		if (opt_yield) {
			sched_yield();
		}
		t2 = t1 + value;
	} while(__sync_val_compare_and_swap(pointer, t1, t2) != t1);
}


void *thread_func(void* arg) {
	struct threadArgs *t =  (struct threadArgs*) arg;
	long long *counter = t->c;
	int iter = t->iterations;
	int i;
	char* s_opt = t->sync_opt;
	for (i=0;i<iter;i++) {
		switch(*s_opt) {
			case 'n':
				add(counter,(long long)1);
				break;
			case 'm':
				add_with_mutex(counter,(long long)1);
				break;
			case 's':
				spin_lock_add(counter,(long long)1);
				break;
			case 'c':
				comp_and_swap_add(counter,(long long)1);
				break;
		}
		
	}

	for (i=0;i<iter;i++) {
		switch(*s_opt) {
			case 'n':
				add(counter,(long long)-1);
				break;
			case 'm':
				add_with_mutex(counter,(long long)-1);
				break;
			case 's':
				spin_lock_add(counter,(long long)-1);
				break;
			case 'c':
				comp_and_swap_add(counter,(long long)-1);
				break;
		}
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	//counter = 0;
	opt_yield = 0;
	long long operationTime;
	operationTime = 0;
	int numThreads =1 ;
	int numIterations =1;
	long long counter = 0;
	char syncOption = 'n';
	long long starttime;
	long long endtime;
	
	struct timespec times;
	clock_gettime(CLOCK_MONOTONIC,&times);
	
	//endtime = (double) times.tv_sec + (double)  times.tv_nsec * 0.000000001;
	//fprintf(stdout, "%f\n", starttime);

	struct option long_options[] = {
		{"yield", no_argument,&opt_yield,1},
		{"threads",optional_argument,0,'t'},
		{"iterations",optional_argument,0,'i'},
		{"sync",required_argument,0,'s'},
		{NULL,0,NULL,0}
	};
	int index = 0;
	int c = 0;
	while (c != -1){
		opterr= 0;
		c = getopt_long(argc,argv,"",long_options,&index);
		switch (c) {
			case 't':
				if (optarg != NULL) {
					numThreads = atoi(optarg);
				}
				//fprintf(stdout, "%d Threads\n", numThreads);
				break;
			case 'i':
				if (optarg != NULL) {
					numIterations = atoi(optarg);
				}
				//fprintf(stdout, "%d Iterations\n", numIterations);
				break;
			case 's':
				syncOption = optarg[0];
				break;
			case '?':
				fprintf(stderr, "Invalid Option!\n" );
				exit(1);
				break;
			
		}
	}
	//printf("%c\n",syncOption);
	starttime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;
	//Got options
	pthread_t* thread_array = malloc(numThreads*sizeof(pthread_t));

	//counter =100;
	//long long x = 1;
	int i;
	for (i = 0;i<numThreads;i++) {
		struct threadArgs t;
		t.c = &counter;
		t.iterations = numIterations;
		t.sync_opt = &syncOption;
		pthread_create(&thread_array[i], NULL,thread_func,&t);
	}
	for (i=0;i<numThreads;i++) {
		pthread_join(thread_array[i],NULL);
	}
	printf("add-");
	if (opt_yield) {
		printf("yield-");
	}
	if (syncOption == 'm' || syncOption == 's' || syncOption == 'c') {
		printf("%c,", syncOption);
	}
	else
	{
		printf("none,");
	}

	
	clock_gettime(CLOCK_MONOTONIC,&times);
	endtime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;
	operationTime = (endtime-starttime)/(numThreads*numIterations*2);
	printf("%d,",numThreads);
	printf("%d,", numIterations);
	printf("%d,", numThreads*numIterations*2);
	printf("%lld,", endtime - starttime);
	printf("%lld,", operationTime);
	printf("%lld\n", counter);
	//fprintf(stdout, "Total time: %lld\n", endtime - starttime);
	return 0;
}