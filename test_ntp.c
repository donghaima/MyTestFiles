#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


typedef uint64_t abs_time_t;


static inline abs_time_t timeval_to_abs_time(struct timeval tv)
{
    abs_time_t result;
    result = ((uint64_t)tv.tv_sec)*1000000 + tv.tv_usec;
    return result;
}


static inline abs_time_t get_sys_time(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, 0) == -1) {
        perror("gettimeofday");
    }

    return timeval_to_abs_time(tv);
}

int main(void)
{
    int i, total=100;
    abs_time_t abs_ts[100];

    /* Get 1000 NTP timestamps as fast as we can */
    for (i=0; i<total; i++) {
        //ntp_ts = get_ntp_time();
        //abs_ts[i] = ntp_to_abs_time(ntp_ts);
        abs_ts[i] = get_sys_time();
    }

    /* Show the diffs against the first timestamp */
    for (i=0; i<total; i++) {
        printf("diff[%d] = %llu\n", i, abs_ts[i]-abs_ts[0]);
    }

    return 0;
}
