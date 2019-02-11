CC=gcc


build: add list

add:	lab2_add.c
	gcc -Wall -Wextra -pthread -o lab2_add lab2_add.c

list: lab2_list.c SortedList.c SortedList.h
	gcc -Wall -Wextra -pthread -o lab2_list lab2_list.c SortedList.c SortedList.h

clean: 
	rm lab2_add lab2_list  errs.txt lab2a-204585474.tar.gz

tests: build
	bash ./makecsvfiles.sh

graphs: build tests
	- ./lab2_add.gp 
	- ./lab2_list.gp

dist: graphs lab2_add.gp lab2_list.gp lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.gp lab2_list.gp 
	./lab2_add.gp
	./lab2_list.gp
	tar -czvf lab2a-204585474.tar.gz lab2_add.c lab2_list.c SortedList.h SortedList.c Makefile makecsvfiles.sh \
	lab2_add.csv lab2_list.csv README lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.gp lab2_list.gp