#include <stdio.h>
#include <stdint.h>

typedef void *avsm_fa_cookie_t;

#define SIZE 10
#define MAX_HASH_BUCKETS  0x400
#define MAX_HASH_MASK    (MAX_HASH_BUCKETS -1)

/*
 * Identifies a single IPv4 flow
 */
typedef struct avsm_fa_flow_id_ipv4_t_ {
    uint64_t dest_addr;
    uint64_t src_addr;
    uint16_t dest_port;
    uint16_t src_port;
    uint8_t protocol;
    uint8_t pad1[3];
    avsm_fa_cookie_t client_cookie; /* would be queue pointer for KFA */
} avsm_fa_flow_id_ipv4_t;

static unsigned hash(avsm_fa_flow_id_ipv4_t  ipv4FlowId)
{
#if 1
    //uint64_t hashValue = ipv4FlowId.src_port << 16 + ipv4FlowId.dest_port;
    uint64_t hashValue = ipv4FlowId.src_port + ipv4FlowId.dest_port;
    hashValue += ipv4FlowId.src_addr;
    hashValue += ipv4FlowId.dest_addr;
    hashValue  = hashValue & MAX_HASH_MASK;
    //hashValue  = hashValue % MAX_HASH_MASK;
#else
    uint32_t hashValue = ipv4FlowId.src_port << 16 + ipv4FlowId.dest_port;
    printf("hashValue=%lx\n", hashValue);
    hashValue += (uint32_t)ipv4FlowId.src_addr;
    printf("hashValue=%lx\n", hashValue);
    hashValue += (uint32_t)ipv4FlowId.dest_addr;
    printf("hashValue=%lx\n", hashValue);
    hashValue  = hashValue & MAX_HASH_MASK;
    printf("hashValue=%lu\n", hashValue);
#endif
    return hashValue;
}


int main (void)
{
    int i;
    avsm_fa_flow_id_ipv4_t fid[SIZE] = {
        { 0xd010302, 0x10040402, 7778, 7778, 17, 0, 0},  // hash 772
        { 0x5031b02, 0x10040102, 256, 7777,  17, 0, 0},   // hash 4
        { 0x5031b02, 0x10040202, 256, 7777,  17, 0, 0},   // hashIndex 260
        { 0x5031b02, 0x10040302, 256, 7777,  17, 0, 0},   // hashIndex 516
        { 0x5031b02, 0x10040402, 256, 7777,  17, 0, 0},   // hashIndex 772

        { 0xd010302, 0x10040302, 57005, 48879, 17, 0, 0},  // hashIndex 516
        { 0xd010302, 0x10040302, 57053, 48879, 17, 0, 0},  // hashIndex 516
        { 0xd010302, 0x10040302, 7778,  7778,  17, 0, 0},  // hashIndex 516
        { 0xd010202, 0x10040402, 57005, 48879, 17, 0, 0},  // hashIndex 516
        { 0xd010202, 0x10040402, 57053, 48879, 17, 0, 0}, // hashIndex 516

    };

    for (i=0; i< SIZE; i++) {

        printf("Hash value for %lx:%u-%lx:%u-%u-%x is %lu\n",
               fid[i].src_addr,  fid[i].src_port, 
               fid[i].dest_addr, fid[i].dest_port, 
               fid[i].protocol,  fid[i].client_cookie, hash(fid[i]));
    }

    return 0;
}
