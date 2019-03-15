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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int sfd=0;
uint16_t value;
time_t rawtime;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
mraa_aio_context temperature;
int celsius = 0;
int logging = 0;
char* logFileName = NULL;
int period=1;
int stopped;
FILE* logfile;
int shutdownFlag=0;
int portNum;
char* id;
char* hostName;
struct tm *localTime;
char writebuf[1024];

void button_handler() {
	if (shutdownFlag == 0) {
		shutdownFlag = 1;
	}
}

void sigint_handler(int sig) {
	fprintf(stderr,"Interrupt Received %d\n",sig);
	mraa_aio_close(temperature);
	close(sfd);

	exit(0);
}

void printTemp() {
	while (1) {
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
		   	if (logging) {
		   		bzero(writebuf,1024*sizeof(char));
				int b =0;
		   		b += sprintf(writebuf,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
		   		b += sprintf(writebuf+b, "%.1f\n",temp);
		   		write(sfd,writebuf,b);
		   	}
	   	}
	   	if (shutdownFlag) {
	   		if (logging) {
	   			time(&rawtime);
				localTime = localtime(&rawtime);
				bzero(writebuf,1024*sizeof(char));
				int b =0;
		   		b += sprintf(writebuf,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
				b += sprintf(writebuf+b,"SHUTDOWN\n");
				write(sfd,writebuf,b);
			}
			
			exit(0);	   		
	   	}
	   	sleep(period);
   }
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
		if (shutdownFlag==0) {
			shutdownFlag = 1;
		}
	} else {
		if (logging && !shutdownFlag) {
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
	
	int idArg = 0;
	int hostArg =0;
	struct option long_options[] = {
		{"scale",required_argument,0,'s'},
		{"period",required_argument,0,'p'},
		{"log",required_argument,0,'l'},
		{"id",required_argument,0,'i'},
		{"host",required_argument,0,'h'},
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
			case 'i':
				idArg = 1;
				id = optarg;
				break;
			case 'h':
				hostArg = 1;
				hostName = optarg;
				break;
			case '?':
				fprintf(stderr,"Invalid argument!\n");
				exit(1);				
		}		
	}
	if (argv[optind] == NULL) {
		fprintf(stderr, "Port number arg missing\n");
		exit(1);
	}
	else {
		portNum = atoi(argv[optind]);
		if (portNum <= 0) {
			fprintf(stderr, "Invalid portNum arg\n");
			exit(1);
		}
		//printf("Port=%d\n", portNum);
	}
	if (!idArg || !hostArg || !logging) {
		fprintf(stderr, "Missing mandatory args\n");
		exit(1);
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

	//printf("about to connect\n");
	//get socket stuff
	sfd = socket(AF_INET,SOCK_STREAM,0);
	struct hostent *h;
	h=gethostbyname(hostName);
	//printf("Got hostname\n");
	struct sockaddr_in address;
	bzero(&address,sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(portNum);
	memcpy(&address.sin_addr,h->h_addr,h->h_length);
	if (connect(sfd,(struct sockaddr*)&address,sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Unable to connect to server\n");
		exit(1);
	}

	//printf("CONNECTED!\n");
	///printf("%s\n", id);
	writebuf[1024];
	int b=0;
	b += sprintf(writebuf,"ID=%s\n",id);
	write(sfd,writebuf,b);
	//printf("Wrote to server!\n");
	pthread_t tempReporting;

	if (pthread_create(&tempReporting,NULL,(void*)printTemp,NULL) <0 ) {
		fprintf(stderr, "Error creating temp thread\n");
		exit(1);
	}
	while (1) {
		offset = 0;
		char readbuf[1024];
		char currWord[1024];
		read(sfd,readbuf,1024);

		for (i=0;i<1024;i++){
			currWord[i-offset] = readbuf[i];

			if (readbuf[i] == '\n' || readbuf[i] <= 0){ //hit end of command
				currWord[i-offset] = '\0';

				evaluateWord(currWord);
				
				//printf("Currword=%s\n", currWord);
				offset = (i+1);
				
			}
			if (shutdownFlag) {
				time(&rawtime);
				localTime = localtime(&rawtime);
				value = mraa_aio_read(temperature);
				float R = 1023.0/value-1.0;
			    R = R0*R;
			    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
			     if (!celsius) {
			    	temp = 1.8*temp + 32;
			    }
			   	if (logging) {
			   		bzero(writebuf,1024*sizeof(char));
					int b =0;
			   		b += sprintf(writebuf,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
			   		b += sprintf(writebuf+b, "%.1f\n",temp);
			   		write(sfd,writebuf,b);
			   	}
		   		if (logging) {
					bzero(writebuf,1024*sizeof(char));
					int b =0;
			   		b += sprintf(writebuf,"%02d:%02d:%02d ",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
					b += sprintf(writebuf+b,"SHUTDOWN\n");
					write(sfd,writebuf,b);
				}
				exit(0);	  
			} 		
   	
			//printf("%c",buf[i]);
		}

	}
	close(sfd);
	// }
	if (logging) {
		fclose(logfile);
	}
	mraa_aio_close(temperature);
	return 0;
}
