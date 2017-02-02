#include <stdio.h>
#include <stdint.h>

int main (int argc, char* argv[])
{
    uint32_t a = 400000;
    uint32_t b = 1356, c = 0;
    float f = 0.;
    
    if (argc == 2) 
        b = atoi(argv[1]);

    c = a*8./b;
    f = (double)a/(double)b;

    printf("a = %u, b = %u, a/b=%u, float=a/b=%f\n", a, b, c, f);

    return 0;
}
