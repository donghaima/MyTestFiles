#include <stdio.h>
#include <stdint.h>

int main(void)
{
	uint8_t name[4] = "CSCO";

	printf("name[]=%d, %d, %d, %d\n", name[0], name[1], name[2], name[3]); 	
	return 0;
}
