
CC=gcc

default: lab1c.c
	gcc -o simpsh lab1c.c

clean: 
	rm simpsh lab1-204585474.tar.gz

check: default


dist: lab1c.c README
	tar -czvf lab1-204585474.tar.gz lab1c.c makefile README report.pdf
