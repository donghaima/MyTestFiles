                
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>

int main(int argc, char *argv[])
{
	cap_t cap = cap_get_proc();

	if (!cap) {
		perror("cap_get_proc");
		exit(1);
	}
	printf("%s: running with caps %s\n", argv[0], cap_to_text(cap, NULL));
	cap_free(cap);
	return 0;
}
