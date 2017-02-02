#include <stdio.h>
#include <stdint.h>

/* Test the sizes for different structures */

typedef struct fcc_tlv_t_ {
    uint32_t start_seq_number;
    uint32_t start_rtp_time;
    uint32_t dt_earliest_join;
    uint32_t dt_repair_end;
    uint8_t *tsrap;
    uint16_t tsrap_len;
} fcc_tlv_t;


typedef struct fcc_tlv_t2_ {
    uint32_t start_seq_number;
    uint32_t start_rtp_time;
    uint32_t dt_earliest_join;
    uint32_t dt_repair_end;
    uint16_t tsrap_len;
    uint8_t tsrap[0];
} fcc_tlv_t2;

int main(void)
{
	fcc_tlv_t t1 = {};
	fcc_tlv_t2 t2 = {};
        uint8_t buf[100];

	fcc_tlv_t2 *p_t2 = NULL;
        int buff_len = sizeof(buf);
        p_t2 = malloc (sizeof(fcc_tlv_t2) + sizeof(uint8_t)*buff_len);
        memcpy(p_t2->tsrap, &buf[0], sizeof(buf));

	fcc_tlv_t2 *p_t2_2 = NULL;
        p_t2_2 = malloc (sizeof(fcc_tlv_t2) + sizeof(uint8_t *));
        //p_t2_2->tsrap = &buf[0];

	printf("Sizeof uint8_t:%d; uint16_t:%d; uint32_t:%d; uint64_t:%d\n",
               sizeof(uint8_t), sizeof(uint16_t), 
               sizeof(uint32_t), sizeof(uint64_t));

	printf("Sizeof char:%d; short:%d; int:%d; long:%d; unsigned long:%d\n",
               sizeof(char), sizeof(short), sizeof(int),
               sizeof(long), sizeof(unsigned long));

	printf("Sizeof fcc_tlv_t is: %d\n", sizeof(fcc_tlv_t));
	printf("Sizeof fcc_tlv_t2 is: %d\n", sizeof(fcc_tlv_t2));

        free(p_t2);
        free(p_t2_2);

	return 1;
}
