#/bin/bash

echo ""
echo "Making csv file"
if [ -e "lab2_add.csv" ]
then
	rm "lab2_add.csv"
fi
touch "lab2_add.csv"
if [ -e lab2add ]
then
	echo "Executable exists, continuing"
else
	exit 1
fi
iter=(100 1000 10000 100000)

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i >>lab2_add.csv
	done
done

echo "Ran 4 add-none tests with 100,1000,10000,100000 iterations"
echo ""
echo "Running tests with --yield options"
iter=(10 20 40 80 100 1000 10000 100000)

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --yield>>lab2_add.csv
	done
done

echo "Finished running --yield tests"
echo ""
echo "Running tests with --sync=m options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=m >>lab2_add.csv
	done
done

echo "Finished running sync=m tests"
echo ""
echo "Running tests with --sync=m --yield options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=m --yield >>lab2_add.csv
	done
done

echo "Finished running --sync=m --yield tests"
echo ""
echo "Running tests with --sync=s options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=s >>lab2_add.csv
	done
done

echo "Finished running sync=s tests"
echo ""
echo "Running tests with --sync=s --yield options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=s --yield >>lab2_add.csv
	done
done

echo "Finished running sync=s --yield tests"
echo ""
echo "Running tests with --sync=c options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=c >>lab2_add.csv
	done
done

echo "Finished running sync=c tests"
echo ""
echo "Running tests with --sync=c --yield options"

for i in ${iter[@]}; do
	for j in {1..20}; do
			./lab2add --threads=$j --iterations=$i --sync=c --yield >>lab2_add.csv
	done
done

echo "Finished running --sync=c --yield tests"

./lab2_add.gp


echo "Creating csv file for lab2list now"
if [ -e "lab2_list.csv" ]
then
	rm "lab2_list.csv"
fi
touch "lab2_list.csv"
if [ -e lab2list ]
then
	echo "Executable exists, continuing"
else
	exit 1
fi

echo "Running tests 1 thread, iterations"
iter=(10 100 1000 10000 20000)
for i in ${iter[@]}; do
	./lab2list --iterations=$i >>lab2_list.csv
done
if [ -s errs.txt ]
then
	echo "Errors Occured"
fi
echo "Finished Running Iterations tests with 1 thread"

echo "Running multiple threads and iterations to see failure"
iter=(1 10 100 1000)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j >>lab2_list.csv 2>>errs.txt
	done
done
echo "Finished multiple threads and iterations to see failure"
echo ""
echo "Various yield options for errors"
echo "yield=i"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=i>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=i"
echo ""
echo "yield=d"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=d>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=d"
echo ""
echo "yield=il"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=il>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=il"
echo ""
echo "yield=dl"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=dl>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=dl"

echo "Do above yield tests with mutex protection"
echo "yield=i"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=i --sync=m>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=i"
echo ""
echo "yield=d"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=d --sync=m>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=d"
echo ""
echo "yield=il"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=il --sync=m>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=il"
echo ""
echo "yield=dl"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=dl --sync=m>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=dl"
echo "Completed mutex protection tests"

echo "Do above yield tests with spinlock protection"
echo "yield=i"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=i --sync=s>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=i"
echo ""
echo "yield=d"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=d --sync=s>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=d"
echo ""
echo "yield=il"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=il --sync=s>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=il"
echo ""
echo "yield=dl"
iter=(1 2 4 8 16 32)
threads=(2 4 8 12)
for i in ${iter[@]}; do
	for j in ${threads[@]}; do
		./lab2list --iterations=$i --threads=$j --yield=dl --sync=s>>lab2_list.csv 2>>errs.txt
	done
done
echo "Done yield=dl"
echo "Completed spinlock protection tests"

echo "No yield mutex protection tests"
threads=(2 4 8 12 16 24)
for j in ${threads[@]}; do
		./lab2list --iterations=1000 --threads=$j --sync=m>>lab2_list.csv 2>>errs.txt
done
echo "Finished no yield mutex protection tests"
echo ""
echo "No yield spinlock protection tests"
threads=(2 4 8 12 16 24)
for j in ${threads[@]}; do
		./lab2list --iterations=1000 --threads=$j --sync=s>>lab2_list.csv 2>>errs.txt
done
echo "Finished no yield spinlock protection tests"

./lab2_list.gp

