CC=gcc

default: lab4b.c
	gcc -lmraa -o lab4b lab4b.c -lm

check: test.c
	gcc -lmraa -o t test.c
	./t

clean: lab4b t
	rm lab4b t *.txt

dist: lab4b.c README Makefile test.c
	tar -czvf lab4b-204585474.tar.gz lab4b README Makefile test.c

