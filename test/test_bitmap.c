#include <stdio.h>
#include <stdint.h>


int main(void)
{
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


    return 0;
}
