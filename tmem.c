/* Utility to print running total of VmPeak and VmSize of a program */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PATH_MAX 2048

time_t startTS;

static int main_loop(char *pidstatus)
{
	char *line;
	char *vmsize;
	char *vmpeak;
	char *vmrss;
	char *vmhwm;
	char *vmdata, *vmstk;
	
	size_t len;
	
	FILE *f;

	vmsize = NULL;
	vmpeak = NULL;
	vmrss = NULL;
	vmhwm = NULL;
	vmdata = vmstk = NULL;

	line = malloc(128);
	len = 128;
	
	f = fopen(pidstatus, "r");
	if (!f) return 1;
	
	/* Read memory size data from /proc/pid/status */
	while (!vmsize || !vmpeak || !vmrss || !vmhwm || !vmdata || !vmstk)
	{
		if (getline(&line, &len, f) == -1)
		{
			/* Some of the information isn't there, die */
			return 1;
		}
		
		/* Find VmPeak */
		if (!strncmp(line, "VmPeak:", 7))
		{
			vmpeak = strdup(&line[7]);
		}
		
		/* Find VmSize */
		else if (!strncmp(line, "VmSize:", 7))
		{
			vmsize = strdup(&line[7]);
		}
		
		/* Find VmRSS */
		else if (!strncmp(line, "VmRSS:", 6))
		{
			vmrss = strdup(&line[7]);
		}
		
		/* Find VmHWM */
		else if (!strncmp(line, "VmHWM:", 6))
		{
			vmhwm = strdup(&line[7]);
		}
		
		/* Find VmData */
		else if (!strncmp(line, "VmData:", 7))
		{
			vmdata = strdup(&line[7]);
		}
		
		/* Find VmStk */
		else if (!strncmp(line, "VmStk:", 6))
		{
			vmstk = strdup(&line[7]);
		}
	}
	free(line);
	
	fclose(f);

	/* Get rid of " kB\n"*/
	len = strlen(vmsize);
	vmsize[len - 4] = 0;
	len = strlen(vmpeak);
	vmpeak[len - 4] = 0;
	len = strlen(vmrss);
	vmrss[len - 4] = 0;
	len = strlen(vmhwm);
	vmhwm[len - 4] = 0;
	len = strlen(vmdata);
	vmdata[len - 4] = 0;
	len = strlen(vmstk);
	vmstk[len - 4] = 0;
	
	/* Output results to stderr */
	fprintf(stderr, "%ld %s %s %s %s %s %s\n",
		time(NULL)-startTS, 
		vmsize, vmpeak, vmrss, vmhwm, vmdata, vmstk);
	
	free(vmpeak);
	free(vmsize);
	free(vmrss);
	free(vmhwm);
	free(vmdata);
	free(vmstk);
	
	/* Success */
	return 0;
}


int main(int argc, char **argv)
{
	char buf[PATH_MAX];
	
	if (argc < 2) {
	  printf("usage: %s <pid>\n", argv[0]);
	  return -1;
	}

	int pid = atoi(argv[1]);
	
	snprintf(buf, PATH_MAX, "/proc/%d/status", pid);
	
	/* Continual scan of proc */
	startTS = time(NULL);
	fprintf(stderr, "startTS(%d) vmsize vmpeak vmrss vmhwm vmdata vmstk\n",
		startTS);

	while (!main_loop(buf))
	{
		/* Wait for 1 sec */
		usleep(1000000);
	}
	
	return 1;
}
