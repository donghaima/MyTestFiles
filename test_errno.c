#include <stdio.h>
#include <string.h>
#include <errno.h>

char *strerror(int errnum);

int main (void)
{
    int errnum;

    for (errnum = 0; errnum < 120; errnum++) {
        printf("errnum=%d, strerror=%s\n", errnum, strerror(errnum));
    }

    return 0;
}
