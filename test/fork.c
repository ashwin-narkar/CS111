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
	int s = fork() < fork() ;
	printf("Return value: %d\n",s);
	
	exit(0);
}
