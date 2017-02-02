/* Test the get_cycles() in test_timex.h 
 * 
 * gcc -g -DCONFIG_686 test_rdtsll.c -o test_rdtsll
 */

#include <stdio.h>
#include <unistd.h>

#include "test_timex.h"


int main (void)
{
    cycles_t c1, c2;
    c1 = get_cycles();

    usleep(100);

    c2 = get_cycles();
    
    printf("c1=%llu, c2=%llu, c2 - c1 = %llu\n", c1, c2, c2-c1);

    return 0;
}
