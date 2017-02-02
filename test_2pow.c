#include <stdio.h>


unsigned int next_highest_power_of_2 (unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v += (v==0);

    return v;
}

unsigned short next_highest_power_of_2_short (unsigned short s)
{
    s--;
    s |= s >> 1;
    s |= s >> 2;
    s |= s >> 4;
    s |= s >> 8;
    s++;
    s += (s==0);

    return s;
}
int main (int argc, char *argv[])
{
    unsigned int v = 123;
    unsigned short s = 123;
    
    if (argc > 1) {
        v = atoi(argv[1]);
        s = atoi(argv[1]);
    }
        
    printf("input number v = %u, the next highest power of 2 = %u\n",
           v, next_highest_power_of_2(v));

    printf("input number s = %u, the next highest power of 2-short = %u\n",
           s, next_highest_power_of_2_short(s));

    return 0;
}
