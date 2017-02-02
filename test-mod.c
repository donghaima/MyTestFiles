#include <stdio.h>
#include <stdint.h>

int main (void)
{
    int i;
    uint16_t toValue[6]={100, 100,200,200,300,300};
    for (i=1; i<=5; i++) {
        printf("i=%d %d %d %d\n", 
                i, i-i%2, 100*(i-i%2), toValue[i]);
    }

}
