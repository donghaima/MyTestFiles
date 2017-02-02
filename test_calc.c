#include <stdio.h>
#include <stdint.h>
#include "vam_time.h"

int main (void)
{
    uint32_t DJ = 100, lag, Q = 5;
    int Lmin;
    uint32_t fastfill_time_ms = 3000;
    float e = 3.;

    lag = 2000;
    rel_time_t fastfill_time = TIME_MK_R(msec, fastfill_time_ms);
    
    /* Lmin = (lag + (1-e)*Delta_J)/e */
    Lmin = (100*lag + (100.-e)*DJ)/e + Q;

    printf("%s: lag=%u(ms), e=%f, DJ=%u(ms), Q=%u(ms), Lmin=%u\n",
           __func__, lag, e, DJ, Q, Lmin);

    Lmin += e / (100. + e) * (TIME_GET_R(msec, fastfill_time));
    printf("%s: fastfill adjusted Lmin=%u(ms)\n", __func__, Lmin);

    return 0;
}
