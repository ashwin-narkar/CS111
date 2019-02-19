#include <stdio.h>
#include <stdlib.h>

typedef double number;


int hash(char* key) {
	if (key == "")
		return -1;
	int avg = 2;
	int i=0;
	while (key[i] != '\0') {
		avg = avg*3 + (key[i]);
		i++;		
	}
	
	return avg % 4;
}
 
int main(int argc, char *argv[]) {
	fprintf(stdout, "Hello World\n");
	char* s = "aklsdjh";
	if (argc > 1) {
		printf("%d\n", hash(argv[1]));
	}
	
	exit(0);
}
