#include <stdio.h>

/* A safe maximum macro that operates on any arithmetic type and 
 * evaluates each of its arguments exactly once
 */
#define max(a,b)               \
    ({ typeof (a) _a = (a);     \
        typeof (b) _b = (b);    \
        _a > _b ? _a : _b; })

#define min(a,b)                \
    ({ typeof (a) _a = (a);     \
        typeof (b) _b = (b);    \
        _a < _b ? _a : _b; })


#define unsafe_max(X,Y) ((X) < (Y) ? (X) : (Y))
#define unsafe_min(X,Y) ((X) < (Y) ? (X) : (Y))


int main (void)
{
    int a=5, b=7;

    printf("a=%d, b=%d, min(a,b)=%d\n", a, b, min(a,b));
    printf("a=%d, b=%d, max(a,b)=%d\n", a, b, max(a,b));

    a=5; b=7;
    printf("a=%d, b=%d, min(a++, b++)=%d\n", a, b, min(a++, b++));
    a=5; b=7;
    printf("a=%d, b=%d, max(a++, b++)=%d\n", a, b, max(a++, b++));

    a=5; b=7;
    printf("a=%d, b=%d, us_min(a,b)=%d\n", a, b, unsafe_min(a,b));
    printf("a=%d, b=%d, us_max(a,b)=%d\n", a, b, unsafe_max(a,b));

    a=5; b=7;
    printf("a=%d, b=%d, us_min(a++,b++)=%d\n", a, b, unsafe_min(a++, b++));
    printf("a=%d, b=%d, us_max(a++,b++)=%d\n", a, b, unsafe_max(a++, b++));

    return 0;
}
