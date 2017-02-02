#include <stdio.h>

/* */
float cvt(unsigned short value)
{
    return (value >> 0xF) + (value & 0x7FFF)/(float)(1 << 0xF);
}

int main()
{
    unsigned short data = 0;

    for(data = 0; data <= 0x8000; ++data)
        printf("data == 0x%X : as float == %f\n", data, cvt(data));

    return 0;
}
