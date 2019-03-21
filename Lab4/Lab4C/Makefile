CC=gcc

default: tcp tls

tcp: lab4c_tcp.c
	gcc -pthread -lmraa -Wall -Wextra -o lab4c_tcp lab4c_tcp.c -lm 

tls: lab4c_tls.c
	gcc -pthread -lmraa -o lab4c_tls lab4c_tls.c -lm -lssl -lcrypto

run_tcp: lab4c_tcp
	./lab4c_tcp --id=420691234 --host=lever.cs.ucla.edu --log=LOGFILE.txt 18000

run_tls: lab4c_tls
	./lab4c_tls --id=420691234 --host=lever.cs.ucla.edu --log=LOGFILE.TXT 19000

clean: 
	rm -f lab4c_tcp lab4c_tls *.txt lab4c-204585474.tar.gz

dist: lab4c_tcp.c README lab4c_tls.c
	tar -czvf lab4c-204585474.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
