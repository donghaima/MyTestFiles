#include <stdio.h>
#include <stdint.h>
#include <unistd.h>


/**@brief
 * Cloned from rdtsc() as in cm_common.h.
 * 
 * A function to read the timestamp counter register
 * on the x86.  Use with caution because: (i) may differ
 * by core; (ii) rate may change due to powersave; (iii) 
 * may behave very differently on different chips; (iv) 
 * takes something like 100 cycles to read on intel chips.
 * Function returns the current value of the Time Stamp Counter (TSC)
 * which is a free-running counter that runs at the system clock rate.
 */
static inline uint64_t vqe_rdtsc (void) {
  uint32_t lo, hi;
 /* We cannot use "=A", since this would use %rax on x86_64 */
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (uint64_t)hi << 32 | lo;
}


int main (int argc, char *argv[])
{
    uint64_t start_tics = vqe_rdtsc();
    int usecs = 1000;

    if (argc > 1)
        usecs = atoi(argv[1]);

    printf("usleep for %d usecs\n", usecs);
    usleep(usecs);

    uint64_t stop_tics = vqe_rdtsc();
    printf("Measured by TSC: %llu tics, scaled by 2.33G: %f\n", 
           stop_tics - start_tics, ((double)(stop_tics - start_tics))/2331.);

    while (1) {
        printf("TSC tics: %llu\n", vqe_rdtsc());
        sleep(1);
    }
    return 0;
}
