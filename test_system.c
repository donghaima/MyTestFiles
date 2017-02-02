#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	system("/sbin/syslogd -m 0 -n &");

	printf("After the system() call\n");
	system("logger -p local0.info \"hello world\"");
	return 0;
}
