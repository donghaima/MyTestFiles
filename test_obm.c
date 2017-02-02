#include <stdio.h>
#include <stdint.h>


int main (void)
{
   int64_t rcc_bw_budget;
   uint8_t outbw_alloc_percent = 95;
   
   uint64_t avail_outbw_mbps = 4736;
   
   uint64_t reserved_er_outbw_bps = 543200000;

   /* rcc_bw_budget = avail_outbw_mbps*1000000 * outbw_alloc_percent/100
    *                           - reserved_er_outbw_bps
    */
   rcc_bw_budget = avail_outbw_mbps * 10000 * outbw_alloc_percent
       - reserved_er_outbw_bps;

   printf("Calculated rcc_bw_budget = %llu(Mbps)\n", rcc_bw_budget/1000000);

   if (rcc_bw_budget < 0) {
       rcc_bw_budget = 0;
   }

   printf("Capped rcc_bw_budget = %llu\n", rcc_bw_budget);
       
   return 0;
}
