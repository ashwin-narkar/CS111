#include <stdlib.h>	/* needed to define exit() */
#include <unistd.h>	/* needed for fork() and getpid() */
#include <stdio.h>	/* needed for printf() */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h> 	//for profile flag



void forkFunction(int x) {
	int pid = fork();
	switch (pid) {
		case 0:

			sleep(1);
			fprintf(stdout, "This the child from call %d. waddup fool\n\n",x);
			fflush(stdout);
			exit(0);
			break;
		case -1:
			fprintf(stdout, "We have a problem sir\n");
			exit(1);
			break;
		default:
			
			fprintf(stdout, "This the parent process from call %d: pid=%d\n\n", x, getpid());
			fflush(stdout);
			break;
	}
}

int main(int argc, char **argv) {
	int pid;	/* process ID */
	int s;
	int runningProcesses = 0;
	int finishedProcesses = 0;
	struct rusage profileUsage;
	double start_user_time;
	double end_user_time;
	double total_user_time;
	double start_cpu_time;
	double end_cpu_time;
	double total_cpu_time;
	double overall_user_start_time;

double overall_cpu_start_time;
	// int u = getrusage(RUSAGE_SELF,&profileUsage);
	// printf("%ld.%06ld\n", profileUsage.ru_utime.tv_sec, profileUsage.ru_utime.tv_usec);
	// printf("%ld.%06ld\n", profileUsage.ru_stime.tv_sec, profileUsage.ru_stime.tv_usec);
	// start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
	// start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
	for (int i =1;i<=5;i++) {
		forkFunction(i);
		runningProcesses++;
	}
	while (finishedProcesses != runningProcesses) {
		int x = waitpid(-1,&s,0);
		fprintf(stdout, "Process %d waited for\n", x);
		finishedProcesses++;
	}
	int u = getrusage(RUSAGE_SELF,&profileUsage);
	// end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
	// end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
	// total_user_time = end_user_time - start_user_time;
	// total_cpu_time = end_cpu_time - start_cpu_time;
	// fprintf(stdout, "User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
	double user =  profileUsage.ru_utime.tv_sec;
	double useru = profileUsage.ru_utime.tv_usec;
	printf("%ld.%06ld\n", profileUsage.ru_utime.tv_sec, profileUsage.ru_utime.tv_usec);
	fprintf(stdout, "Finished running\n");	


	exit(0);
}
