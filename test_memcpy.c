#include <stdio.h>
#include <stdint.h>
#include <string.h>


void foo (uint8_t *data, int len)
{
    uint32_t u1 = 0;
    memcpy(&u1, data, len);
}

int main (void)
{
    uint8_t correct[] = {0x01, 0x02, 0x03, 0x04, };	
    uint8_t large[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    };

    printf("Good data test\n");
    foo(correct, sizeof(correct));
    printf("Returned back\n");

    printf("Now try some rouge data\n");
    foo(large, sizeof(large));
    printf("Returned back\n");

    return 0;	
}
