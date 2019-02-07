
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "fcntl.h"


int main(int argc, char* argv[]) {
	
	struct option long_options[] = {
		{"input",required_argument,0,'i'},
		{"output",required_argument,0,'o'},
		{"segfault",no_argument,0,'s'},
		{"catch",no_argument,0,'c'},
		{"dump-core",no_argument,0,'d'},
		{NULL,0,NULL,0}
	};