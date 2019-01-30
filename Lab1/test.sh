#!/bin/bash

#check contents of the directory
if [ ! -e simpsh ] 
then
	echo "Error: simpsh doesn't exist"
fi

if [ ! -e makefile ]
then
	echo "Error: no make file"
fi


#test using input and output file
lsTest=`ls`
touch i
touch o
touch e
./simpsh --rdonly i --wronly o --wronly e --command 0 1 2 ls
if [[ $(< o)=$lsTest ]] 
then
	echo "input output file test: Successful"
fi

#test a command that needs arguments
#test using tr command
rm i
rm o
rm e
touch o
touch e
for x in `seq 1 10`;
do
	echo "a" >> i
done
tr "a" "A" < i > testFile
./simpsh --rdonly i --wronly o --wronly e --command 0 1 2 tr "a" "A"
cmp o testFile 
if [ $? -eq 0 ] 
then
	echo "tr test successful"
fi


#test verbose flag

