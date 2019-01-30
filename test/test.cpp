#include <iostream>

using namespace std;

typedef int (*intFunPointer)(int);

int f(int x) {
	return (x+1)*(x+1);
}

void fill_with_values(int a[], int size, intFunPointer f) {
	for (int i=0;i<size;i++) {
		a[i] = (*f)(i);
	}
}

int main() {
	int* x = new int[10];
	fill_with_values(x,10,f);
	for (int i=0;i<10;i++) {
		cout << x[i] << endl;
	}
	delete [] x;
	return 0;
}