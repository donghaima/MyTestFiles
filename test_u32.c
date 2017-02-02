#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint16_t u16 = 0x0102;
    uint32_t u32 = (uint32_t)u16;

    printf("u16 = 0x%.2x, converted to u32=0x%04x\n", u16, u32);

    return 1;
}
