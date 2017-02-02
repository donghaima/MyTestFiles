#include <stdio.h>
#include <stdint.h>


int main(void)
{
    int i;
    uint16_t bit_map = 0x04;
    uint16_t next_send_seq_num = 0;

    printf("Initial values: bitmap=0x%x, next_send_seq_num=%d\n", 
           bit_map, next_send_seq_num);

    while (bit_map) {

        do {
            bit_map >>= 1;
            next_send_seq_num++;

            printf("bitmap=0x%x, next_send_seq_num=%d\n", 
                   bit_map, next_send_seq_num);
        } while (bit_map && ! (bit_map & 0x1));
    }

    /* Given a number n where  1 <= n <= 16, compute a M whose binary form 
     * contains n number of 1.  For example if n = 3, M would be 0b111.
     */
    for (i=1; i<=16; i++) {
        bit_map = (2 << (i-1)) - 1;
        printf("i=%d, computed bitmap=0x%x\n", i, bit_map);
    }

    return 0;
}
