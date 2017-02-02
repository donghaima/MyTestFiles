#include <stdio.h>
#include <stdint.h>

int main (void)
{
    int i = 2, j = 0;
    int k_int;
    uint32_t k_uint32;
    uint8_t k_uint8;

    printf("begin\n");
    k_int = (float) i / j;
    k_uint32 = (float)i / j;
    k_uint8 = (float)i / j;
    printf("after\n");
    printf("i = %i, j=%d, k_int=i/j=%d, k_uint32=%u, k_uint8=%u\n",
           i, j, k_int, k_uint32, k_uint8);
    
    return 0;
}

