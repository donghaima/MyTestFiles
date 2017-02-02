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
        /* Repeatedly shift the number to the right, until the result is zero.
         * Keep counting the number of shifts this requires.
         */
        for (i=(number>>1), position=0; i!= 0; ++position)
            i >>= 1;

        /* The position of the most significant set bit is the number of
         * the shifts we needed after the first one
         */
        result = position;
    }

    return 0;
}
