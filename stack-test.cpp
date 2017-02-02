#include <iostream>

using namespace std;

void r (int l)
{
	unsigned char B[1048576];
	for (int i=0; i<1048576; i++) B[i] = i%256;

	cout << l+1 << "MB on stack" << endl;
	r(l+1);
}

int main (void)
{
	r(0);
}
