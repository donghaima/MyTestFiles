#include <stdio.h>
#include <stdint.h>
#include <assert.h>


unsigned int bsf(unsigned int register bits) {
#ifdef WIN32
    __asm{ 
        xor eax, eax
        bsf eax, bits
    }
#else
    register unsigned int result;
    asm (   
        ".intel_syntax noprefix;"
        "xor    eax, eax;"
        "bsf    eax, %1;"
        ".att_syntax prefix"
        : "=&a"(result)
        : "r"(bits)
        : "flags"
    );

    return result;
#endif
}

unsigned int bsr(unsigned int register bits) {
#ifdef WIN32
    __asm{ 
        xor eax, eax
        bsr eax, bits
    }
#else
    register unsigned int result;
    asm (   
        ".intel_syntax noprefix;"
        "xor    eax, eax;"
        "bsr    eax, %1;"
        ".att_syntax prefix"
        : "=&a"(result)
        : "r"(bits)
        : "flags"
    );

    return result;
#endif
}

uint64_t rdtsc() {
#ifdef WIN32
    __asm {
        rdtsc
    }
#else
    struct {
        union {
            uint64_t   timestamp;
            struct {
                unsigned long low;
                unsigned long high;
            };
        };
    } result;
    asm (   
        ".intel_syntax noprefix;"
        "rdtsc;"
        ".att_syntax prefix"
        : "=a"(result.low), "=d"(result.high)
    );

    return result.timestamp;
#endif
}


#define PAGESIZE 8192

#define INTMASK  31
#define INTSHIFT 5
#define INTCOUNT (PAGESIZE / sizeof(unsigned int))
#define NOFREE   0xFFFFFFFF

unsigned int map[INTCOUNT];


void set_bit(unsigned short index, unsigned int mask)
{
    printf("In set_bit(): block index=%u, mask=0x%x\n", index, mask);
    map[index] |= mask;
}

void clear_bit(unsigned short index, unsigned int mask) 
{
    printf("In clear_bit(): block index=%u, mask=0x%x\n", index, mask);
    map[index] &= ~mask;
}




int main (int argc, char* argv[])
{
    unsigned short size = 2048;

    unsigned short index = 123;  // index to bit btw 0 and 2048

    switch (argc)
    {
        case 3:
           size = atoi(argv[2]);
        case 2:   
           index = atoi(argv[1]);
        default:
           break;
    }   
   
    printf("Use bit index:%u, size:%u\n", index, size);

    unsigned int map_shift_size = bsf(size);

    printf("size=%u, map_shift_size=bsf(%u)=%u, 1<<map_shift_size=%u, "
           "bsr(%u)=%u\n",
           size, size, map_shift_size, (1<<map_shift_size), size, bsr(size));

    assert(index < (1 << map_shift_size));

    printf("To call set_bit(), bit index=%u\n", index);
    set_bit(index>>INTSHIFT, 1 << (index & INTMASK));

    return 0;
}
