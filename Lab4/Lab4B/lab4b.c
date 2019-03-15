#include <string.h>
#include <stdio.h>
#include <math.h>
#include <mraa/aio.h>
#include <signal.h>
#include <time.h>
#include <poll.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include "mraa.h"


const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
mraa_aio_context temperature;
int celsius = 0;
int logging = 0;
char* logFileName = NULL;
int period=1;
int stopped;
FILE* logfile;
int shutdown=0;

void button_handler() {
	if (shutdown == 0) {
		shutdown = 1;
	}
}

void sigint_handler(int sig) {
	fprintf(stderr,"Interrupt Received %d\n",sig);
	mraa_aio_close(temperature);
	exit(0);
}

int getNewPeriod(const char* b) {
	int x = atoi(b+7);
	//printf("%d\n",x);
	char c[8];
	int i;
	for (i=0;i<7;i++){
		c[i] = b[i];
	}
	c[7] = '\0';
	//printf("%s\n",c);
	if (strcmp(c,"PERIOD=")==0) {
		return x;
	}
	else return -999;
}

void evaluateWord(char* buf) {
	if (strcmp(buf,"SCALE=F")==0) {
		if (logging) {
			fprintf(logfile, "SCALE=F\n");
		}
		celsius=0;
	} else if (strcmp(buf,"SCALE=C")==0) {
		if (logging) {
			fprintf(logfile, "SCALE=C\n");
		}
		celsius = 1;
	} else if (strcmp(buf,"START") == 0) {
		if (logging) {
			fprintf(logfile, "START\n");
		}
		if (stopped) {
			stopped = 0;
		}
	} else if (strcmp(buf,"STOP") == 0) {
		if (logging) {
			fprintf(logfile, "STOP\n");
		}
		if (!stopped) {
			stopped = 1;
		}
	} else if(strcmp(buf,"OFF") == 0) {
		if (logging) {
			fprintf(logfile, "OFF\n");

		}
		if (shutdown==0) {
			shutdown = 1;
		}
	} else {
		if (logging && !shutdown) {
			fprintf(logfile, "%s\n", buf);
		}
		int x = getNewPeriod(buf);
		if (x > 0) {
			period = x;
			
		}
	}
}

int main(int argc, char* argv[]) {
	printf("MRAA version: %s\n", mraa_get_version());
	uint16_t value;
	time_t rawtime;
	
	struct option long_options[] = {
		{"scale",required_argument,0,'s'},
		{"period",required_argument,0,'p'},
		{"log",required_argument,0,'l'},
		{NULL,0,NULL,0}
	};
	int index =0;
	int c = 0;
	while (c != -1){
		opterr= 0;
		c = getopt_long(argc,argv,"",long_options,&index);
		switch(c) {
			case 's':
				if (strcmp("F",optarg)==0) {
					//printf("Fahrenheit\n");
					celsius = 0;
				} else if (strcmp("C",optarg)==0) {
					//printf("Celsius\n");
					celsius=1;
				}
				else {
					fprintf(stderr,"Invalid option for scale\n");
					exit(1);
				}
				break;
			case 'p':
				period = atoi(optarg);
				if (period < 1) {
					fprintf(stderr, "Bad period\n");
					exit(1);
				}
				break;
			case 'l':
				logging = 1;
				logFileName = optarg;
				//printf("Logging for file %s\n", logFileName);
				break;
			case '?':
				fprintf(stderr,"Invalid argument!\n");
				exit(1);				
		}		
	}
	temperature = mraa_aio_init(1);
	mraa_gpio_context button = mraa_gpio_init(60);
	mraa_gpio_dir(button, MRAA_GPIO_IN);
	mraa_gpio_isr(button,MRAA_GPIO_EDGE_FALLING,&button_handler,NULL);
	if (temperature == NULL) {
		fprintf(stderr,"ERROR INTIALIZING\n");
		mraa_deinit();
		exit(0);
	}
	signal(SIGINT,sigint_handler);
	struct tm *localTime;
	struct pollfd polling;
	polling.fd = 0;
	polling.events = 0|POLLIN;
	stopped=0;
	
	
	int i=0;
	int offset = 0;
	
	if (logging) {
		logfile = (FILE*)fopen(logFileName,"w");
	}

	if (logfile == NULL) {
		fprintf(stderr, "Unable to open file\n");
		exit(1);
	}
	
	while (1) {
		offset = 0;
		poll(&polling,1,period*1000);
		if (polling.revents & POLLIN) {
			//printf("Stdin happened\n");
			char buf[1024];
			char currWord[1024];
			read(0,buf,1024);

			for (i=0;i<1024;i++){
				currWord[i-offset] = buf[i];

				if (buf[i] == '\n' || buf[i] <= 0){ //hit end of command
					currWord[i-offset] = '\0';

					evaluateWord(currWord);
					
					//printf("Currword=%s\n", currWord);
					offset = (i+1);
					
				}
				if (shutdown) {
					time(&rawtime);
					localTime = localtime(&rawtime);
					value = mraa_aio_read(temperature);
					float R = 1023.0/value-1.0;
				    R = R0*R;
				    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
				     if (!celsius) {
				    	temp = 1.8*temp + 32;
				    }
				    printf("%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
				   	printf("%.1f\n",temp);
				   	if (logging) {
				   		fprintf(logfile,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
				   		fprintf(logfile, "%.1f\n",temp);
				   	}
			   		if (logging) {

				   		fprintf(logfile,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
					
						fprintf(logfile,"SHUTDOWN\n");
						
					}

					printf("%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
					printf("SHUTDOWN\n");
					exit(0);	  
				} 		
	   	
				//printf("%c",buf[i]);
			}

			//printf("%s\n",buf);
			//fflush(stdin);
			//fflush(stdout);
		
			//printf("%d\n",period);
			continue;
		}
		
		time(&rawtime);
		localTime = localtime(&rawtime);
		value = mraa_aio_read(temperature);
		float R = 1023.0/value-1.0;
	    R = R0*R;
	    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
	     if (!celsius) {
	    	temp = 1.8*temp + 32;
	    }
	    if (!stopped) {
		    printf("%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
		   	printf("%.1f\n",temp);
		   	if (logging) {
		   		fprintf(logfile,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
		   		fprintf(logfile, "%.1f\n",temp);
		   	}
	   	}
	   	if (shutdown) {
	   		if (logging) {
	   			time(&rawtime);
				localTime = localtime(&rawtime);
		   		fprintf(logfile,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
				fprintf(logfile,"SHUTDOWN\n");
			}
			printf("%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
			printf("SHUTDOWN\n");
			exit(0);	   		
	   	}

		//sleep(period);
	}
	if (logging) {
		fclose(logfile);
	}
	mraa_aio_close(temperature);
	return 0;
}
