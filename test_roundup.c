#include <stdio.h>
#include <linux/log2.h>

int main (int argc, char *argv[])
{
    /**
     * roundup_pow_of_two - round the given value up to nearest power of two
     * @n - parameter
     *
     * round the given value up to the nearest power of two
     * - the result is undefined when n == 0
     * - this can be used to initialise global variables from constant data
     */
    //#define roundup_pow_of_two(n)
    int n = 1234;
    if (argc > 1)
        n = atoi(argv[1]);

    printf("n=%d, roundup_pow_of_two(n)=%d\n", n, roundup_pow_of_two(n));
    return 0;
}
