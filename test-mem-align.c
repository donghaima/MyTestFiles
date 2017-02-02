#include <stdio.h>

struct test {
	int sn;
	char name[5];  /* will be padded to align memory */
	int amount;
};

int main (void)
{
	printf("size of struct test: %d\n", sizeof(struct test));
	return 0;

}
