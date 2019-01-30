#NAME: Ashwin Narkar
#EMAIL: ashwin.narkar@gmail.com
#ID: 204585474

CC=gcc
CFLAGS=-g -Wall -Wextra


default: lab0.o
	gcc lab0.o $(CFLAGS) -o lab0 

lab0.o: lab0.c
	gcc $(CFLAGS) -c lab0.c -o lab0.o

clean:
	-rm -f lab0 lab0.o lab0-204585474.tar.gz

check: 

dist: lab0.c breakpoint.png backtrace.png 
	tar -czvf lab0-204585474.tar.gz lab0.c breakpoint.png backtrace.png Makefile README
