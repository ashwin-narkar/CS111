CC=gcc


build: list


list: lab2_list.c SortedList.c SortedList.h
	gcc -g -Wall -lprofiler -Wextra -pthread -o lab2_list lab2_list.c SortedList.c SortedList.h

clean: 
	rm lab2_list err.txt lab2b-204585474.tar.gz 

profile: lab2_list
	-rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libpriofiler.so
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_func ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

tests: build
	bash ./makecsvfiles.sh

graphs: tests
	./lab2_list.gp

dist: graphs profile lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png makecsvfiles.sh README lab2b_list.csv lab2_list.gp lab2_list2.csv
	tar -czvf lab2b-204585474.tar.gz lab2_list.c SortedList.c SortedList.h Makefile profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png makecsvfiles.sh README lab2b_list.csv lab2_list.gp lab2_list2.csv