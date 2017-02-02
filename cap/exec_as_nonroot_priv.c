#include <sys/prctl.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

void printmycaps(void)
{
	cap_t cap = cap_get_proc();

	if (!cap) {
		perror("cap_get_proc");
		return;
	}
	printf("%s\n",  cap_to_text(cap, NULL));
	cap_free(cap);
}

int main(int argc, char *argv[])
{
	cap_t cur;
	int ret;
	int newuid;

	if (argc<4) {
		printf("Usage: %s <uid> <capset>"
			"<program_to_run>\n", argv[0]);
		exit(1);
	}
	ret = prctl(PR_SET_KEEPCAPS, 1);
	if (ret) {
		perror("prctl");
		return 1;
	}
	newuid = atoi(argv[1]);
	printf("Capabilities before setuid: ");
	printmycaps();
	ret = setresuid(newuid, newuid, newuid);
	if (ret) {
		perror("setresuid");
		return 1;
	}
	printf("Capabilities after setuid, before capset: ");
	printmycaps();
	cur = cap_from_text(argv[2]);
	ret = cap_set_proc(cur);
	if (ret) {
		perror("cap_set_proc");
		return 1;
	}
	printf("Capabilities after capset: ");
	cap_free(cur);
	printmycaps();
	ret = execl(argv[3], argv[3], NULL);
	if (ret)
		perror("exec");
}
