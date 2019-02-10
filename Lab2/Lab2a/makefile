CC=gcc


default: add list

add:	lab2_add.c
	gcc -Wall -Wextra -pthread -o lab2add lab2_add.c

list: lab2_list.c SortedList.c SortedList.h
	gcc -Wall -Wextra -pthread -o lab2list lab2_list.c SortedList.c SortedList.h

clean: 
	rm lab2add lab2list 

tests_add: default
	bash ./makecsvfiles.sh

dist: default