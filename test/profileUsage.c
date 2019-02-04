#include <stdlib.h>	/* needed to define exit() */
#include <unistd.h>	/* needed for fork() and getpid() */
#include <stdio.h>	/* needed for printf() */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h> 	//for profile flag

struct rusage usage;

double end_cpu_time;
double end_user_time;

double overall_user_start_time;
double overall_cpu_start_time;

int main(int argc, char **argv) {
	int u = getrusage(RUSAGE_SELF,&usage);
	overall_user_start_time = (double) usage.ru_utime.tv_sec + (double)  usage.ru_utime.tv_usec * 0.000001;
	overall_cpu_start_time = (double) usage.ru_stime.tv_sec + (double)  usage.ru_stime.tv_usec * 0.000001;
	// fprintf(stderr, "%f\n", overall_user_start_time);
	// fprintf(stderr, "%f\n", overall_cpu_start_time);
	//sleep(2);
	int i,j; for (i=0,j=0; i<100000000; i++) { 
		j+=i*i; 
	}  
	u = getrusage(RUSAGE_SELF,&usage);
	// end_user_time = (double) usage.ru_utime.tv_sec + (double)  usage.ru_utime.tv_usec * 0.000001;
	// end_cpu_time = (double) usage.ru_stime.tv_sec + (double)  usage.ru_stime.tv_usec * 0.000001;
	fprintf(stderr, "%ld\n", usage.ru_utime.tv_sec);
	fprintf(stderr, "%f\n", (double) (usage.ru_utime.tv_usec*0.000001));
	fprintf(stderr, "%ld\n", usage.ru_stime.tv_sec);
	fprintf(stderr, "%ld\n", usage.ru_stime.tv_usec);
	
	// fprintf(stderr, "%f\n", overall_cpu_start_time);
	exit(0);
}
