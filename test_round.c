#include <stdio.h>
#include <math.h>
#include <stdint.h>

int main (void)
{
    double d1 = 12.34, d2 = 23.67, d3 = 0.23, d4= 0.65;
    uint32_t ci, fi, ri;

    ci = ceil(d1);
    fi = floor(d1);
    ri = round(d1);
    printf("d1=%f ceil(d1)=%d floor(d1)=%d\n", d1, ci, fi);

    ci = ceil(d2);
    fi = floor(d2);
    printf("d2=%f ceil(d2)=%d floor(d2)=%d\n", d2, ci, fi);

    ci = ceil(d3);
    fi = floor(d3);
    printf("d3=%f ceil(d3)=%d floor(d3)=%d\n", d3, ci, fi);

    ci = ceil(d4);
    fi = floor(d4);
    printf("d4=%f ceil(d4)=%d floor(d4)=%d\n", d4, ci, fi);

    return 0;
}
