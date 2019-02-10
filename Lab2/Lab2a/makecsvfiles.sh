#/bin/bash
echo ""
echo "Making csv file"
if [ -e "lab2_add.csv" ]
then
	rm "lab2_add.csv"
fi
touch "lab2_add.csv"
if [ -e lab2a ]
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