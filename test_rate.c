#include <stdio.h>
#include <stdint.h>

int main(void) 
{
    int i;
    uint32_t curr_er_requests = 0xffffff00, last_er_reqs;
    float curr_er_rate;   /* number of repairs per second */

    last_er_reqs = curr_er_requests;

    for (i=0; i<300; i++) {
        curr_er_requests += 10;
        curr_er_rate = (curr_er_requests - last_er_reqs) / 5.;

        printf ("i=%d, curr_er_requests=%u; curr_er_rate=%.2f\n",
                i, curr_er_requests, curr_er_rate);

        if (curr_er_rate < 0.) {
            /* Wrapped: 858993459 = 0xffffffff / 5. */
            curr_er_rate += 858993459;
            printf ("curr_er_rate < 0, change to %.2f\n", curr_er_rate);
        }
        
        last_er_reqs = curr_er_requests;
        printf ("New last_er_reqs = %u\n", last_er_reqs);
    }
 
    return 0;
}
