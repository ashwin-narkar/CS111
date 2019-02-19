#/bin/bash

: '
NAME: Ashwin Narkar
ID: 204585474
EMAIL: ashwin.narkar@gmail.com
'

echo ""
echo "Making csv file"
if [ -e "lab2b_list.csv" ]
then
	rm "lab2b_list.csv"
fi
touch "lab2b_list.csv"
if [ -e lab2_list ]
then
	echo "Executable exists, continuing"
else
	exit 1
fi
iter=(100 1000 10000 100000)
threads=(1 2 4 8 12 16 24)
for j in ${threads[@]}; do
	./lab2_list --threads=$j --iterations=1000 --sync=m >>lab2b_list.csv 2>>err.txt
done
for j in ${threads[@]}; do
	./lab2_list --threads=$j --iterations=1000 --sync=s >>lab2b_list.csv 2>>err.txt
done

echo "Multiple sublist testing"
threads=(1 2 4 8 12 16)
iter=(1 2 4 8 16)
for j in ${threads[@]}; do
	for i in ${iter[@]}; do
		./lab2_list --threads=$j --iterations=$i --lists=4 --yield=id >>lab2b_list.csv 2>>err.txt || true
	done
done

echo "4 lists with sync m and s"
iter=(10 20 40 80)
for j in ${threads[@]}; do
	for i in ${iter[@]}; do
		./lab2_list --threads=$j --iterations=$i --lists=4 --yield=id --sync=m >>lab2b_list.csv 2>>err.txt 
	done
done
for j in ${threads[@]}; do
	for i in ${iter[@]}; do
		./lab2_list --threads=$j --iterations=$i --lists=4 --yield=id --sync=s >>lab2b_list.csv 2>>err.txt 
	done
done

if [ -e "lab2_list2.csv" ]
then
	rm "lab2_list2.csv"
fi
touch "lab2_list2.csv"

echo "Rerunning synchronized versions no yield"
threads=(1 2 4 8 16)
lists=(1 4 8 16)
for j in ${threads[@]}; do
	for i in ${lists[@]}; do
		./lab2_list --threads=$j --iterations=1000 --lists=$i --sync=m >>lab2_list2.csv 2>>err.txt
	done
done
for j in ${threads[@]}; do
	for i in ${lists[@]}; do
		./lab2_list --threads=$j --iterations=1000 --lists=$i --sync=s >>lab2_list2.csv 2>>err.txt 
	done
done

echo "Finished"
