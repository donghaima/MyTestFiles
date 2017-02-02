#include <stdio.h>
#include <stdint.h>

int main (void)
{
    int32_t count, count_prio;
    int32_t diff, i;
    
    count = 0xffffff00;
    count_prio = count - 13;

    for (i = 0; i < 260; i++) {
        count += 3;

        diff = count - count_prio;

        if (diff) {
            printf("diff = %lu, count = %lu, count_prio = %lu\n",
                   diff, count, count_prio);
            
            /* update */
            count_prio = count;

        } else {
            printf("Should not be here!\n");
        }
    }

    printf("\n\n");

    uint16_t rsp_seq_num = 65530;
    uint16_t outstanding_reqs = 2;
    uint16_t nxt_req_seq_num = 65532;
    uint16_t calc_nxt_req_seq_num;
    uint16_t calc_rsp_seq_num;

    for (i = 0; i < 10; i++) {
        //nxt_req_seq_num = rsp_seq_num + outstanding_reqs;

        calc_nxt_req_seq_num = (uint16_t)(rsp_seq_num + outstanding_reqs);
        calc_rsp_seq_num = (uint16_t)(nxt_req_seq_num - outstanding_reqs);

        printf("i=%d, rsp_seq_num=%u, outstanding_reqs=%u, "
               "nxt_req_seq_num=%u\n"
               "calc_rsp_seq_num=%u, calc_nxt_req_seq_num=%u\n",
               i, rsp_seq_num, outstanding_reqs, nxt_req_seq_num,
               calc_rsp_seq_num, calc_nxt_req_seq_num);

        if (calc_nxt_req_seq_num != nxt_req_seq_num) {
            printf("Error-1: i=%d, rsp_seq_num=%u, outstanding_reqs=%u, "
                   "nxt_req_seq_num=%u\n",
                   i, rsp_seq_num, outstanding_reqs, nxt_req_seq_num);
        }

        if (calc_rsp_seq_num != rsp_seq_num) {
            printf("Error-2: i=%d, rsp_seq_num=%u, outstanding_reqs=%u, "
                   "nxt_req_seq_num=%u\n",
                   i, rsp_seq_num, outstanding_reqs, nxt_req_seq_num);
        }

        rsp_seq_num ++;
        nxt_req_seq_num ++;
    }
    
}
