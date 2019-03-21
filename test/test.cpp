#include <iostream>

using namespace std;

int main(void)
{
	int value;
	value = 1;
	cout << "Preincrement operator value " << ++value << endl;
	cout << "Value = " << value << endl;
	cout << "Post increment opreator value " << value++ << value++ << endl;
	cout << "Value = " << value << endl;
	cout << "Post increment opreator 2 value " << value++ << endl;
	cout << "Value = " << value << endl;
}
