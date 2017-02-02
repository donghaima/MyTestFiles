#include <stdio.h>

#define BITS_PER_BYTE 8
#define DEFAULT_PAK_SIZE 1356

int main (void)
{
    int i;
    int result, rate_percent = 5;
    
    for (i=1; i<10000001; i++) {
        //result = (float)(rate_percent / 100) * i / (BITS_PER_BYTE * DEFAULT_PAK_SIZE); 
        result = (float)(rate_percent / 100) * i / BITS_PER_BYTE / DEFAULT_PAK_SIZE; 
    }
    
}
