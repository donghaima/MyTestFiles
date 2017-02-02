#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char cmd[32];
    int ret;

    if (argc < 2) {
        fprintf(stderr, "Need to specify an address\n");
        return -1;
    }

    /* Add addr to lo */
    snprintf(cmd, 32, "addr add %s/32 dev lo", argv[1]);
    //ret = system(cmd);
    
    ret = execve("/sbin/ip", cmd, NULL);

    if (ret < 0) {
        perror("execve()");
        return -1;
    }

    printf("Run execev(): %s; ret=%d; ip addr show dev lo:\n", 
           cmd, ret);

    system("ip addr show dev lo");

    return 0;
}
