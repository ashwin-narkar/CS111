#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h> 	//for profile flag

//Add Profile flag
//Profile flag for all options except the file opening ones


struct command {
	int id;
	char** arguments;	//array of strings for arguments
	int numarg;
	int status; //for wait_flag
	int argmax;	//for dynamic memory reallocation

};

int argumentindex;
int c;
int* files;
int maxFiles;
int filenumber;
char* filename;
int newfile;
int verbose_flag;
int fileFlags;
int sig;
int wait_flag;
int error_flag;
int max_exit_code;
int max_exit_signal;
int runningProcesses;
int maxProcesses;
int exited_with_signal;
int finishedProcesses;
pid_t* commandIds;
struct command *commands;
int profile_flag;
struct rusage profileUsage;
double start_user_time;
double end_user_time;
double total_user_time;
double start_cpu_time;
double end_cpu_time;
double total_cpu_time;
double overall_user_start_time;

double overall_cpu_start_time;


void crash() {
	int *x = NULL;
	*x = 5;
	return;
}

void catchError(int signum) {
	fprintf(stderr,"%d caught\n",signum);
	exit(signum);
}

struct command getCommandArgs(int argc, char* argv[]) {
	struct command toRun;
	toRun.argmax = 20;
	toRun.numarg = 0;
	toRun.arguments = malloc(toRun.argmax*sizeof(char*));
	argumentindex = optind-1;
	while (argumentindex < argc && (argv[argumentindex][0] != '-' || argv[argumentindex][1] != '-')) {
		toRun.arguments[toRun.numarg] = argv[argumentindex];
		argumentindex++;
		toRun.numarg++;
		if (toRun.numarg >= toRun.argmax) {
			toRun.argmax += 20;
			toRun.arguments = realloc(toRun.arguments, toRun.argmax*sizeof(char*));
		}
	}
	if (toRun.numarg < 4) {
			fprintf(stderr, "Not enough arguments for command\n");

			error_flag = 1;
			return toRun;
		}
	optind = argumentindex;
	if (verbose_flag == 1) {
		fprintf(stdout,"--command");
		for (int i=0;i<toRun.numarg;i++){
			fprintf(stdout," %s",toRun.arguments[i]);

		}
		fprintf(stdout,"\n");
		fflush(stdout);
	}
	return toRun;
}

void runCommand(struct command c) { 

	for (int i=0;i<3;i++) {
		if ((filenumber < 3 && atoi(c.arguments[i]) > 2) || atoi(c.arguments[i]) > (filenumber-1) && filenumber > 3) {
			fprintf(stderr,"Error fd %s doesnt exist\n",c.arguments[i]);
			exit(1);
		}
	}
	
	//TODO: ADD FSTAT CHECK HERE
	//only if it has been opened before
	struct stat b;
	//if 2 has been given to command and files havent been opened then its fine

	for (int i=0;i<c.numarg;i++) {
		if (fstat(files[atoi(c.arguments[i])],&b) < 0) {
			//fprintf(stderr, "File descriptor to command is closed\n");
			error_flag =1;
		}
	}
	//fprintf(stdout, "About to fork for command %s\n",c.arguments[3]);
	commands[runningProcesses] = c;
	commandIds[runningProcesses] = fork();
	int pid = commandIds[runningProcesses];
	if (pid < 0 ) {
		fprintf(stderr, "Fork error!\n");
		exit(errno);
	}
	if (pid == 0) { //dup2 files in the child process so that it doesnt override it in the parent process
		c.id = getpid();
		//fprintf(stdout, "In child for %s\n", c.arguments[3]);
		if (c.numarg < 4) {
			//fprintf(stderr, "Not enough arguments for command\n");
			//error_flag = 1;
			exit(1);
		}
		//fprintf(stdout, "Finna dup2\n");
		for (int i=0;i<3;i++) {
			if (dup2(files[atoi(c.arguments[i])],i) < 0) {
					fprintf(stderr,"Dup2 error for %d descriptor\n",i);
					fprintf(stderr,"error for files[i] = %d\n",files[atoi(c.arguments[i])]);
					exit(1);
			}

			
		}
		for (int i = 0;i<filenumber;i++) {
			close(files[i]);
		}
		//fprintf(stdout,"Got to line 150\n");
		execvp(c.arguments[3],&c.arguments[3]);

	}
	//parent process waits for child if the wait flag is HIGH
	else if (pid>0){
		//fprintf(stdout, "parentprocess for child %d\n", pid);
		runningProcesses++;
		// if (wait_flag) {
		// 	int s;
		// 	fprintf(stdout, "Waiting for child\n");
		// 	pid_t p = waitpid(-1,&s,0);		//all child processes
		// 	fprintf(stdout, "Waiting for children. Still have %d processes\n", p);
		// 	//fprintf(stdout, "finishedProcesses %d \t runningProcesses %d at line 560\n", finishedProcesses, runningProcesses);
		// 	if (WIFEXITED(s)) {
		// 		fprintf(stdout,"exit %d ",WEXITSTATUS(s));
		// 		fflush(stdout);
		// 		if (WEXITSTATUS(s) >= max_exit_code) {
		// 			max_exit_code = WEXITSTATUS(s);
		// 		}
		// 	}
			
		// 	else if (WIFSIGNALED(s) ) 
		// 	{
		// 		fprintf(stdout,"signal %d ",WTERMSIG(s));
		// 		fflush(stdout);
		// 		exited_with_signal = 1;
		// 		if (WTERMSIG(s) > max_exit_signal) {
		// 			max_exit_signal = WTERMSIG(s);

		// 		}
		// 	}
		// 	//int currProcess;
		// 	// for (int i =0;i< runningProcesses;i++) {
		// 	// 	if (commandIds[i] == p) {
		// 	// 		currProcess = i;
		// 	// 		break;
		// 	// 	}
		// 	// }
		// 	for (int i=3;i<c.numarg;i++) {
		// 		fprintf(stdout, "%s ", c.arguments[i]);
		// 	}
		// 	//print arguments here
		// 	fprintf(stdout,"\n");
		// 	finishedProcesses++;
		// }
		if (profile_flag) {
			int u = getrusage(RUSAGE_SELF,&profileUsage);
			end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
			end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
			total_user_time = end_user_time - start_user_time;
			total_cpu_time = end_cpu_time - start_cpu_time;
			fprintf(stdout, "User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
		}
	}

}

int main(int argc, char* argv[]) {

	int qe = getrusage(RUSAGE_SELF,&profileUsage);
	overall_user_start_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
	overall_cpu_start_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
	fprintf(stdout, "overall_user_start_time = %f\n", overall_user_start_time);
	fprintf(stdout, "overall_cpu_start_time = %f\n", overall_cpu_start_time);



	argumentindex = 0;
	c = 0;
	filenumber=0;
	maxFiles = 30;
	maxProcesses = 20;
	verbose_flag = 0;
	wait_flag = 0;
	error_flag = 0;
	runningProcesses = 0;
	finishedProcesses = 0;
	files = malloc(maxFiles*sizeof(int));
	commands = malloc(maxProcesses*sizeof(struct command));
	commandIds = malloc(maxProcesses*sizeof(pid_t));
	max_exit_code = 0;
	exited_with_signal = 0;
	profile_flag = 0;



	struct option long_options[] = {
		{"rdonly",required_argument,0,'r'},	//TODO: Profile Flag
		{"wronly",required_argument,0,'w'}, //TODO: Profile Flag
		{"command",required_argument,0,'c'},	//TODO: Profile Flag
		{"verbose",no_argument,0,'v'},	
		{"append",no_argument,0,'1'},
		{"cloexec",no_argument,0,'2'},
		{"creat",no_argument,0,'3'},
		{"directory",no_argument,0,'4'},
		{"dsync",no_argument,0,'5'},
		{"excl",no_argument,0,'6'},
		{"nofollow",no_argument,0,'7'},
		{"nonblock",no_argument,0,'8'},
		{"rsync",no_argument,0,'9'},
		{"sync",no_argument,0,'0'},
		{"trunc",no_argument,0,'t'},
		{"close",required_argument,0,'l'}, //TODO: Profile Flag
		{"pipe",no_argument,0,'p'},	//TODO: Profile Flag
		{"abort",no_argument,0,'a'},	//TODO: Profile Flag
		{"catch",required_argument,0,'h'},	//TODO: Profile Flag
		{"ignore",required_argument,0,'i'},	//TODO: Profile Flag
		{"default",required_argument,0,'u'},	//TODO: Profile Flag
		{"pause",no_argument,0,'q'},	//TODO: Profile Flag
		{"rdwr",required_argument,0,'d'},	//TODO: Profile Flag
		{"wait",no_argument,0,'z'},	//TODO: Profile Flag
		{"profile",no_argument,0,'b'},	
		{NULL,0,NULL,0}
	};

	for (int i = 0; i < argc; i++) {
	 	if(strcmp("--wait", argv[i]) ==0) {
	    	wait_flag = 1;
	      	break;
	    }
	}
	while (c != -1){
		opterr= 0;
		c = getopt_long(argc,argv,"",long_options,&argumentindex);
		
		switch(c) {
			case '1':
				if (verbose_flag) {
					fprintf(stdout, "--append\n");
				}
				fileFlags |= O_APPEND;
				break;
			case '2': 
				if (verbose_flag) {
					fprintf(stdout, "--cloexec\n");
				}
				fileFlags |= O_CLOEXEC;
				break;
			case '3':
				if (verbose_flag) {
					fprintf(stdout, "--creat\n");
				}
				fileFlags |= O_CREAT;
				break;
			case '4':
			if (verbose_flag) {
					fprintf(stdout, "--directory\n");
				}
				fileFlags |= O_DIRECTORY;
				break;
			case '5':
				if (verbose_flag) {
					fprintf(stdout, "--dsync\n");
				}
				fileFlags |= O_DSYNC;
				break;
			case '6':
				if (verbose_flag) {
					fprintf(stdout, "--excl\n");
				}
				fileFlags |= O_EXCL;
				break;
			case '7':
				if (verbose_flag) {
					fprintf(stdout, "--nofollow\n");
				}
				fileFlags |= O_NOFOLLOW;
				break;
			case '8':
				if (verbose_flag) {
					fprintf(stdout, "--nonblock\n");
				}
				fileFlags |= O_NONBLOCK;
				break;
			case '9':
				if (verbose_flag) {
					fprintf(stdout, "--rsync\n");
				}
				fileFlags |= O_RSYNC;
				break;
			case '0':
				if (verbose_flag) {
					fprintf(stdout, "--sync\n");
				}
				fileFlags |= O_SYNC;
				break;
			case 't':
				if (verbose_flag) {
					fprintf(stdout, "--trunc\n");
				}
				fileFlags |= O_TRUNC;
				break;
			case 'r':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--rdonly %s\n",optarg);
				}
				filename = optarg;
				fileFlags |= O_RDONLY;
				files[filenumber] = open(filename,fileFlags,0644);
				fileFlags = 0;
				if (files[filenumber] < 0) {
					fprintf(stderr, "Error %d\n", errno);
					
				}
				filenumber++;
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--rdonly User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'w':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--wronly %s\n",optarg);
				}
				filename = optarg;
				fileFlags |= O_WRONLY;
				
				files[filenumber] = open(filename,fileFlags,0644);
				fileFlags = 0;
				if (files[filenumber] < 0) {
					fprintf(stderr, "Open Error %d\n", errno);
					
				}
				filenumber++;
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--wronly User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'c':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				runCommand(getCommandArgs(argc, argv));
				break;
			case 'v':
				verbose_flag = 1;
				break;
			case 'l':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag) {
					fprintf(stdout,"--close %s\n",optarg);
				}
				int fd = atoi(optarg);
				if (close(files[fd]) < 0) {
					fprintf(stderr, "Close error: %d\n", errno);
				}
				
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--close User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'p':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag) {
					fprintf(stdout, "--pipe\n");
				}
				int newPipe[2];
				if (pipe(newPipe) < 0 ) {
					fprintf(stderr, "Pipe error: %d\n", errno);
				}
				files[filenumber] = newPipe[0];
				filenumber++;
				files[filenumber] = newPipe[1];
				filenumber++;
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--pipe User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'a':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag) {
					fprintf(stdout, "--abort\n");
					fflush(stdout);
				}
				crash();
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "-- abort User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
					fflush(stdout);
				}
				break;
			case 'h':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag) {
					fprintf(stdout, "--catch %s\n",optarg);
				}
				sig = atoi(optarg);
				signal(sig,catchError);
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--catch User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'd':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--rdwr %s\n",optarg);
				}
				filename = optarg;
				fileFlags |= O_RDWR;
				
				files[filenumber] = open(filename,fileFlags,0644);
				fileFlags = 0;
				if (files[filenumber] < 0) {
					perror("RDWR Error: ");
				}
				filenumber++;
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--rdwr User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'i':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--ignore %s\n",optarg);
				}
				sig = atoi(optarg);
				signal(sig,SIG_IGN);
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--ignore User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'q':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--pause\n");
				}
				pause();
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--pause User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				
				}
				break;
			case 'u':
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				}
				if (verbose_flag){
					fprintf(stdout,"--default %s\n",optarg);
				}
				sig = atoi(optarg);
				signal(sig,SIG_DFL);
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--default User time: %f\tCPU time: %f\n",total_user_time, total_cpu_time);
				}
				break;
			case 'z':
				// if (profile_flag) {
				// 	int u = getrusage(RUSAGE_SELF,&profileUsage);
				// 	start_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
				// 	start_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
				// }
				if (verbose_flag){
					fprintf(stdout,"--wait\n");
				}
				for (int i=0;i<filenumber;i++) {
					close(files[i]);
				}
				finishedProcesses = 0;
				while (finishedProcesses != runningProcesses) {
					//fprintf(stdout, "finishedProcesses %d \t runningProcesses %d\n", finishedProcesses, runningProcesses);
					int s;
					pid_t p = waitpid(-1,&s,0);		//all child processes
					//fprintf(stdout, "Waiting for children. Still have %d processes\n", p);
					//fprintf(stdout, "finishedProcesses %d \t runningProcesses %d at line 560\n", finishedProcesses, runningProcesses);
					
					if (WIFEXITED(s)) {
						fprintf(stdout,"exit %d ",WEXITSTATUS(s));
						fflush(stdout);
						if (WEXITSTATUS(s) >= max_exit_code) {
							max_exit_code = WEXITSTATUS(s);
						}
					}
					
					else if (WIFSIGNALED(s) ) 
					{
						fprintf(stdout,"signal %d ",WTERMSIG(s));
						fflush(stdout);
						exited_with_signal = 1;
						if (WTERMSIG(s) > max_exit_signal) {
							max_exit_signal = WTERMSIG(s);

						}
					}
					int currProcess;
					for (int i =0;i< runningProcesses;i++) {
						if (commandIds[i] == p) {
							currProcess = i;
							break;
						}
					}
					for (int i=3;i<commands[currProcess].numarg;i++) {
						fprintf(stdout, "%s ", commands[currProcess].arguments[i]);
					}
					//print arguments here
					fprintf(stdout,"\n");
					finishedProcesses++;
					
				}
				runningProcesses = 0;
				finishedProcesses = 0;
				if (profile_flag) {
					int u = getrusage(RUSAGE_SELF,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "--wait User time: %f\t\tCPU time: %f\n",total_user_time, total_cpu_time);

					u = getrusage(RUSAGE_CHILDREN,&profileUsage);
					end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
					end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
					total_user_time = end_user_time - start_user_time;
					total_cpu_time = end_cpu_time - start_cpu_time;
					fprintf(stdout, "Child User time: %f\tChild CPU time: %f\n",total_user_time, total_cpu_time);




				}
				break;
			case 'b':
				if (verbose_flag) {
					fprintf(stdout, "--profile\n");
				}
				profile_flag = 1;
				break;
			case '?':
				fprintf(stderr,"Invalid option: %s\n",optarg);
				error_flag = 1;
				break;
		}
		if (filenumber >= maxFiles-1) {		//to account for pipe making 2
			maxFiles += 5;
			files = realloc(files, maxFiles*sizeof(int));
		}
	}

	if (error_flag > max_exit_code) {
		max_exit_code = error_flag;
	}

	// Total running time output
	// 	fails the sanity check but used for report
	// int u = getrusage(RUSAGE_SELF,&profileUsage);
	// fprintf(stderr, "%ld\n", profileUsage.ru_utime.tv_sec);
	// fprintf(stderr, "%ld\n", profileUsage.ru_utime.tv_usec);
	// fprintf(stderr, "%ld\n", profileUsage.ru_stime.tv_sec);
	// fprintf(stderr, "%ld\n", profileUsage.ru_stime.tv_usec);
	// end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
	// end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
	// fprintf(stdout, "Total User time: %f\tTotal CPU time: %f\n",end_user_time, end_cpu_time);
	//fprintf(stdout,"u=%d\n",u);
	if (profile_flag) {
		int u = getrusage(RUSAGE_SELF,&profileUsage);

		end_user_time = (double) profileUsage.ru_utime.tv_sec + (double)  profileUsage.ru_utime.tv_usec * 0.000001;
		end_cpu_time = (double) profileUsage.ru_stime.tv_sec + (double)  profileUsage.ru_stime.tv_usec * 0.000001;
		fprintf(stdout, "Total User time: %f\tTotal CPU time: %f\n",end_user_time, end_cpu_time);
		total_user_time = end_user_time - overall_user_start_time;
		total_cpu_time = end_cpu_time - overall_cpu_start_time;
		
		//fprintf(stdout, "Total User time: %f\tTotal CPU time: %f\n",total_user_time, total_cpu_time);
	}
	if (exited_with_signal) {
		//fprintf(stderr, "Max error signfal%d\n", max_exit_signal);
		exit(max_exit_signal+128);
	}
	else if (max_exit_code) {
		//fprintf(stderr, "Max error code%d\n", max_exit_code);
		exit(max_exit_code);
	}
	if (errno != 0) {
		exit (1);
	}
	
}