#include <stdio.h>

int main (void)
{
    int a = 13;
    switch (a)
    {   
        case 1 ... 10:
        case 12 ... 15:
        printf("a = %d is between 1...10, or 12...15\n", a);
        break;
        
        case 20 ... 50:
        // code
        printf("a = %d is between 20...50\n", a);
        break;
        
        case 800:
        // code
        printf("a = %d is obviously 800\n", a);
        break;
        
        default:
            printf("a = %d is not handled\n", a);
        break;
    }

    return 0;
}
