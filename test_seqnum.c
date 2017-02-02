#include <stdio.h>
#include <stdint.h>

typedef struct cm_seq_num_ {
    uint32_t __num;
} cm_seq_num_t;


static inline cm_seq_num_t 
cm_seq_num_make (uint32_t val)
{
    cm_seq_num_t result;
    result.__num = val;
    return result;
}

typedef uint8_t boolean;

static inline boolean 
cm_seq_num_lt (cm_seq_num_t s0, cm_seq_num_t s1)
{
    return ((s0.__num - s1.__num) >> 31);
}

static inline boolean 
u16_seq_num_lt (uint16_t u16_0, uint16_t u16_1)
{
    uint16_t diff = u16_0 - u16_1;
    return (diff >> 15);
}


int main (void)
{
    uint32_t seqnum_1 = 2, seqnum_2 = 0xfffffffe, seqnum_3 = seqnum_2 - 10, u32_diff;

    uint16_t u16_1 = 2, u16_2 = 0xfffe, u16_3 = 0xfffe, u16_diff;

    printf("seqnum_1=%u, seqnum_2=%u, seqnum_3=%u, "
           "(seqnum_1<seqnum_2) = %d, (seqnum_3<seqnum_2) = %d\n",
           seqnum_1, seqnum_2, seqnum_3,
           cm_seq_num_lt(cm_seq_num_make(seqnum_1), 
                         cm_seq_num_make(seqnum_2)),
           cm_seq_num_lt(cm_seq_num_make(seqnum_3), 
                         cm_seq_num_make(seqnum_2)));

    printf("u16_1=%u, u16_2=%u, u16_3=%u, "
           "(u16_1<u16_2) = %d, (u16_3<u16_2) = %d\n",
           u16_1, u16_2, u16_3,
           u16_seq_num_lt(u16_1, u16_2), u16_seq_num_lt(u16_3, u16_2));


    return 0;
}
