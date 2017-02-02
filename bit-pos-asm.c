#include <stdio.h>

int main (int argc, char* argv[])
{
    long max = 100;
    long number, i;
    unsigned position;

    if (argc > 1)
        max = atoi(argv[1]);

    /* Using volatile variable is to coerce the compiler's optimizer.
     */
    volatile unsigned result;

    /* Repeat the operaiton for a large number */
    for (number = 1; number <= max; ++number) 
    {
        /* Compute the position of the most significant set bit using
         * the bsrl assembly instruction
         */
        asm ("bsrl %1, %0" : "=r" (position) : "r" (number));

        result = position;
    }

    return 0;
}
