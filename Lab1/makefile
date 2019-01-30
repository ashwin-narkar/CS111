
CC=gcc

default: lab1b.c
	gcc -o simpsh lab1b.c

clean: 
	rm simpsh lab1-204585474.tar.gz

check: default test.sh
	./test.sh

dist: lab1b.c README
	tar -czvf lab1-204585474.tar.gz lab1b.c makefile README test.sh