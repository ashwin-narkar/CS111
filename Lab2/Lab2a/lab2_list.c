#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include "fcntl.h"
#include <pthread.h>
#include "SortedList.h"

SortedList_t** list;
SortedList_t* head;
pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0;
char syncOption;

struct threadArgs {
	int threadNum;
	int iter;
};

char* generateRandKey() {
	
	char* allChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	int length = 11;
	char *c = (char*)malloc(length*sizeof(char));
	int i;
	for (i=0;i<length;i++) {
		int x = rand();
		//printf("%d\t", x % 61);
		//printf("%c\n", allChars[x % 61]);
		c[i] = allChars[x % 61];
	}
	c[length-1] = '\0';
	return c;
}

void *thread_func(void* arg) {
	
	

	struct threadArgs *t =  (struct threadArgs*) arg;
	int i;
	int iterations = t->iter;
	int threadID = t->threadNum;
	//fprintf(stdout, "Inserting %d\n", threadID);
	for (i = threadID*iterations; i < threadID*iterations + iterations;i++) {
		//fprintf(stdout, "Inserting %d\n", i);
		
		if (syncOption == 'm') {
			pthread_mutex_lock(&l);
		}
		if (syncOption == 's') {
			while (__sync_lock_test_and_set(&spin_lock,1));
		}

		SortedList_insert(head,list[i]);


		if (syncOption == 's') {
			__sync_lock_release(&spin_lock);
		}

		if (syncOption == 'm') {
			pthread_mutex_unlock(&l);
		}
	}
	int length = SortedList_length(head);
	if (length < 0) {
		fprintf(stderr,"Error with length\n");
		exit(1);
	}
	SortedList_t *foundElement;
	for (i = threadID*iterations; i< threadID*iterations + iterations;i++) {
		//fprintf(stdout, "Finding and deleting %d\n", i);

		if (syncOption == 'm') {
			pthread_mutex_lock(&l);
		}
		if (syncOption == 's') {
			while (__sync_lock_test_and_set(&spin_lock,1));
		}
		foundElement = SortedList_lookup(head,list[i]->key); 

		if (syncOption == 's') {
			__sync_lock_release(&spin_lock);
		}

		if (syncOption == 'm') {
			pthread_mutex_unlock(&l);
		}

		if (foundElement == NULL) {
			fprintf(stderr, "Error in lookup\n");
			exit(1);
		}	


		if (syncOption == 'm') {
			pthread_mutex_lock(&l);
		}
		if (syncOption == 's') {
			while (__sync_lock_test_and_set(&spin_lock,1));
		}

		
		if (SortedList_delete(foundElement) != 0) {
			fprintf(stderr, "Error in delete\n");
			exit(1);
		}

		if (syncOption == 's') {
			__sync_lock_release(&spin_lock);
		}

		if (syncOption == 'm') {
			pthread_mutex_unlock(&l);
		}
	}




	pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
	srand(time(0));
	opt_yield = 0;
	int numThreads =1 ;
	int numIterations =1;
	syncOption = 'n';
	long long starttime;
	long long endtime;
	struct timespec times;
	int i;
	struct option long_options[] = {
		{"threads",optional_argument,0,'t'},
		{"iterations",optional_argument,0,'i'},
		{"sync",required_argument,0,'s'},
		{"yield",required_argument,0,'y'},
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
				if (syncOption != 's' && syncOption != 'm') {
					fprintf(stderr, "Invalid sync option\n");
					exit(1);
				}
				break;
			case 'y':
				//parse optarg to find i,d,l
				if (optarg[0] != 'i' && optarg[0] != 'd' && optarg[0] != 'l') {
					fprintf(stderr, "Invalid argument or no argument for yield provided!\n");
					exit(1);
				}
				while (optarg[i] != '\0') {
					if (optarg[i] == 'i') {
						opt_yield |= INSERT_YIELD;
					}
					if (optarg[i] == 'd') {
						opt_yield |= DELETE_YIELD;
					}
					if (optarg[i] == 'l') {
						opt_yield |= LOOKUP_YIELD;
					}
					i++;
				}
				break;
			case '?':
				fprintf(stderr, "Invalid Option!\n" );
				exit(1);
				break;
			
		}
	}
	
	//Make threads*iterations list elements and store them in some array
	int numElements = numThreads*numIterations;
	list = malloc(numElements*sizeof(SortedList_t*));
	head = (SortedList_t*)malloc(sizeof(SortedList_t));
	head->key = NULL;
	head->prev = head;
	head->next = head;
	
	for (i=0;i<numElements;i++) {
		list[i] = malloc(sizeof(SortedList_t));
		list[i]->prev = NULL;
		list[i]->next = NULL;
		list[i]->key = generateRandKey();
	}


	

	

	// threads*iterations elements
	// each thread had to add numIterations elements
	// the thread argument that is passed should be like
	// a thread number like (thread 0, thread 1,....,thread numThreads)


	int j;
	pthread_t* thread_array = malloc(numThreads*sizeof(pthread_t));
	struct threadArgs* args_array = malloc(numThreads*sizeof(struct threadArgs));

	//Start timing after declaring everything
	clock_gettime(CLOCK_MONOTONIC,&times);
	starttime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;

	for (j=0;j<numThreads;j++) {
		args_array[j].threadNum = j;
		args_array[j].iter = numIterations;
		//fprintf(stdout, "Creating thread %d\n", .threadNum);
		pthread_create(&thread_array[j],NULL,thread_func,&args_array[j]);
	}
	for (j=0;j<numThreads;j++) {
		pthread_join(thread_array[j],NULL);
	}
	//Stop timing after joining
	clock_gettime(CLOCK_MONOTONIC,&times);
	endtime = (long long ) times.tv_sec * 1000000000 + (long long )  times.tv_nsec ;
	long long numOps = numThreads*numIterations*3;
	long long operationTime = ((endtime-starttime)/numOps);

	//fprintf(stdout, "done!\n");
	printf("list-");
	if (opt_yield) {
		if (opt_yield & INSERT_YIELD) {
			printf("i");
		}
		if (opt_yield & DELETE_YIELD) {
			printf("d");
		}
		if (opt_yield & LOOKUP_YIELD) {
			printf("l");
		}
	}
	else {
		printf("none");
	}
	printf("-");
	if (syncOption == 'm' || syncOption == 's') {
		printf("%c,", syncOption);
	}
	else
	{
		printf("none,");
	}
	
	printf("%d,",numThreads);
	printf("%d,",numIterations);
	printf("1,");
	printf("%lld,",numOps);
	printf("%lld,", endtime - starttime);
	printf("%lld\n", operationTime);
	return 0;
}