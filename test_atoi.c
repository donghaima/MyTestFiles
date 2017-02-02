#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Need to specify a number\n");	
		return -1;
	}
	printf("atoi(): %s --> %ld\n", argv[1], atoi(argv[1]));
	printf("strtol(,10): %s --> %ld\n", argv[1], strtol(argv[1], NULL, 10));
	printf("strtol(,16): %s --> %ld\n", argv[1], strtol(argv[1], NULL, 16));

	return 0;
}
