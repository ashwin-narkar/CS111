CC=gcc


default: lab3a.c
	gcc -Wall -Wextra -o lab3a lab3a.c

clean:
	rm lab3a-204585474.tar.gz lab3a

dist: lab3a lab3a.c Makefile README ext2_fs.h
	tar -czvf lab3a-204585474.tar.gz lab3a.c Makefile README ext2_fs.h

