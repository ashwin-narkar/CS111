CC=gcc


build: add list

add:	lab2_add.c
	gcc -Wall -Wextra -pthread -o lab2add lab2_add.c

list: lab2_list.c SortedList.c SortedList.h
	gcc -Wall -Wextra -pthread -o lab2list lab2_list.c SortedList.c SortedList.h

clean: 
	rm lab2add lab2list *.csv *.png

tests: build
	bash ./makecsvfiles.sh

dist: build tests
	tar -czvf lab2_add.c lab2_list.c SortedList.h SortedList.c makefile makecsvfiles.sh \
	lab2_add.csv lab2_list.csv 