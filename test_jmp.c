#include <stdio.h>
#include <setjmp.h>

void first(void);
void second(void);

/* Global "environment" variable; this must be in scope if longjmp is to be called. */
static jmp_buf buf;

int main (int argc, char **argv)
{
    int specific_status = 0;
    switch (setjmp(buf)) {
        case 0: {
            /* Run code that may signal failure via longjmp. */
            printf("calling first\n");
            first();
            printf("first succeeded\n");  /* not reached */
            break;
        }
        case 3:
            specific_status = 3;
            /* fallthrough */
        default: {
            /* Take any action required on failure. */
            if (specific_status != 0)
              printf("first failed, status %d\n", specific_status);
            else
              printf("first failed, status unknown\n");
            break;
        }
    }
    return 0;
}

void first(void)
{
    printf("calling second\n");
    /* If second calls longjmp, this code has no way of knowing,
       so it can't perform any cleanup. */
    second();
    printf("second succeeded\n");  /* not reached */
}

void second(void)
{
    /* Normally the code here would try to do something useful and
       longjmp on failure, but here we're just demonstrating
       longjmp's ability to return from nested function calls.
       Note that this is not as general as a throw in other languages
       since it needs to be aware of the jmp_buf to be used to jump
       back to a specific caller.
       Alternatively the jmp_buf could be in main scope and passed in 
       the function parameters, making error handling more general 
       but complicating function calls. */
    longjmp(buf, 3);
}

