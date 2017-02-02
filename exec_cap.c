#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <sys/types.h>


int setuid_with_caps (int newuid, char *capstr)
{
    int ret;
    cap_t newcaps;

    ret = prctl(PR_SET_KEEPCAPS, 1);
    if (ret) {
        perror("prctl");
        return -1;
    }
    ret = setresuid(newuid, newuid, newuid);
    if (ret) {
        perror("setresuid");
        return -1;
    }
    newcaps = cap_from_text(capstr);
    ret = cap_set_proc(newcaps);
    if (ret) {
        perror("cap_set_proc");
        return -1;
    }
    cap_free(newcaps);

    return 0;
}

int foo ()
{
	return 0;
}


int main (int argc, char *argv[])
{
    int ret;

    if (argc < 2) {
        fprintf(stderr, "Need to specify a program to run\n");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */

        /* Drop capabilities and setuid */
        if (setuid_with_caps(499, "cap_sys_admin=eip") != 0) {
            fprintf(stderr, "setuid_with_caps() failed\n");
            return -1;
        }
        
        ret = execv("/ws/doma/test/test_dscp", argv);

        if (ret < 0) {
            perror("execve()");
            return -1;
        }
        
    } else if (pid > 0) {
        /* Parent process*/
        printf("Run execev(): %s; ret=%d\n", argv[1]);

    } else {
        perror("fork()");
        return (-1);
    }

    foo();
    foo(1);
    foo(1, 2, 3);

    return 0;
}
