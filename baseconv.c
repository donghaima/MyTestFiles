#include <stdio.h>
#include <limits.h>

char *
baseconv (unsigned int num, int base)
{
    /* The size was hard-coded as 33 before.  A better size for the retbuf 
     * array would be sizeof(int)*CHAR_BIT+1.
     */
    static char retbuf[sizeof(int)*CHAR_BIT+1];
    char *p;
    
    if(base < 2 || base > 16)
        return NULL;

    p = &retbuf[sizeof(retbuf)-1];
    *p = '\0';
    
    do {
        *--p = "0123456789abcdef"[num % base];
        num /= base;
    } while(num != 0);

    return p;
}


int main (int argc, char *argv[])
{
    int d = 1234, base;

    if (argc > 1)
        d = atoi(argv[1]);

    for (base = 2; base <= 16; base ++) {
        printf("d=%d, convert to base-%d = %s\n", d, base, baseconv(d, base));
    }

    return 0;
}
