/*
64-bit-isms and fixed sized types

If you have a fixed sized type (eg, uint64_t) and you want to use printf(3) 
to display it, you need to know the real type of the integer. Eg.

    uint64_t x;
    printf("%llu",x);

This however will fail on 64bit machines as uint64_t is long not long long.

POSIX defines some macros to use in this case, normally found in inttypes.h. 
Eg., for an unsigned 64 bit value, use the macro PRIu64. This will be 
substituted for whatever is appropriate on your host C either lu or llu, 
depending on if you are on a 32bit or 64bit host. Eg.

    uint64_t x;
    printf("%"PRIu64,x);

*/

#include <stdio.h>
#include <inttypes.h>

int main (void)
{
    uint64_t x = 64;
    printf("x = 0x%" PRIx64 ", = %" PRIu64 " in test_64bits.c\n", x, x);

    return 0;
}

