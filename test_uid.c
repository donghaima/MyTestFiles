#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main (int argc, char *argv[])
{
    struct passwd pd;
    struct passwd* pwdptr = &pd;
    struct passwd* tempPwdPtr;
    char pwdbuffer[200];
    int  pwdlinelen = sizeof(pwdbuffer);
    char *user = "doma";

    memset(pwdbuffer, 0, sizeof(pwdbuffer));

    if (argc > 1) {
        user = argv[1];
    }

    if ((getpwnam_r(user, pwdptr, pwdbuffer, pwdlinelen, &tempPwdPtr)) 
        != 0) {
        perror("getpwnam_r() error.");
        return (-1);
    } else {
        printf("\nThe user name is: %s\n", pd.pw_name);
        printf("The user id   is: %u\n", pd.pw_uid);
        printf("The group id  is: %u\n", pd.pw_gid);
        printf("The initial directory is:    %s\n", pd.pw_dir);
        printf("The initial user program is: %s\n", pd.pw_shell);
        printf("\npwdline=%s\n", pwdbuffer);
    }

    return (0);
}



