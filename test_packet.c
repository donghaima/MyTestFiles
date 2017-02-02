
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <netinet/ip.h>
#include <netinet/udp.h>


/* STUN request sent locally */
static const uint8_t req_data[] = {
    0x45, 0x00, 0x00, 0x44, 0x00, 0x00, 0x40, 0x00,
    0x40, 0x11, 0xfe, 0x9f, 0x05, 0x03, 0x19, 0x02,
    0x05, 0x03, 0x19, 0x02, 0x2c, 0x64, 0x36, 0xc6,
    0x00, 0x30, 0x3c, 0x4b, 0x00, 0x01, 0x00, 0x14,
    0x21, 0x12, 0xa4, 0x42, 0x73, 0x4f, 0x65, 0x4e,
    0x73, 0x49, 0x48, 0x4e, 0x63, 0x77, 0x70, 0x00,
    0x00, 0x06, 0x00, 0x10, 0xb1, 0x97, 0xfb, 0xc9,
    0x41, 0xad, 0x3e, 0x27, 0x72, 0x9c, 0x05, 0x0f,
    0xd8, 0xbe, 0xfb, 0xf9,
};


/*
static const uint8_t req_data[] = {
    0x45, 0x00, 0x00, 0x44, 0x00, 0x00, 0x40, 0x00, 
    0x3f, 0x11, 0x20, 0x96, 0x05, 0x01, 0x10, 0x02,
    0x05, 0x0f, 0x01, 0x02, 0x87, 0x07, 0x30, 0x39,
    0x00, 0x30, 0x39, 0xda, 0x00, 0x01, 0x00, 0x14,
    0x21, 0x12, 0xa4, 0x42, 0x54, 0x76, 0x5a, 0x63,
    0x66, 0x58, 0x5a, 0x58, 0x54, 0x74, 0x41, 0x00,
    0x00, 0x06, 0x00, 0x10, 0x67, 0xc6, 0x69, 0x73,
    0x51, 0xff, 0x4a, 0xec, 0x29, 0xcd, 0xba, 0xab,
    0xf2, 0xfb, 0xe3, 0x46,
};
*/


/* Play with udp_header->len, and ip_header->tot_len, and ->ihl * /
static const uint8_t req_data[] = {
    0x46, 0x00, 0x00, 0x44, 0x00, 0x00, 0x40, 0x00, 
    0x3f, 0x11, 0x20, 0x96, 0x05, 0x01, 0x10, 0x02,
    0x05, 0x0f, 0x01, 0x02, 0x87, 0x07, 0x30, 0x39,
    0x00, 0x30, 0x39, 0xda, 0x00, 0x01, 0x00, 0x14,
    0x21, 0x12, 0xa4, 0x42, 0x54, 0x76, 0x5a, 0x63,
    0x66, 0x58, 0x5a, 0x58, 0x54, 0x74, 0x41, 0x00,
    0x00, 0x06, 0x00, 0x10, 0x67, 0xc6, 0x69, 0x73,
    0x51, 0xff, 0x4a, 0xec, 0x29, 0xcd, 0xba, 0xab,
    0xf2, 0xfb, 0xe3, 0x46,
};
*/

typedef struct stun_header_t {
  uint16_t  message_type;
  uint16_t  message_length;
  uint32_t  magic_cookie;
  uint8_t   transaction_id[12];
} stun_header_t;



/* Printf a buffer to STDOUT */
void log_raw (char *title, unsigned char *buf, int len)
{
    int i;

    printf("%s:\n", title);

    for (i = 0; i < len; i++) {
        if (i>0 && (i & 15) == 0) {
            printf("\n");
        } else if (i>0 && (i & 7) == 0)
            printf(" ");

        printf(" %02x", buf [i]);
    }
    printf("\n");
}


static uint16_t in_cksum (const uint16_t *addr, uint16_t len, int csum)
{
    int nleft = len;
    const uint16_t *w = addr;
    uint16_t answer;
    int sum = csum;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
        sum += htons(*(u_char *)w<<8);

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);			/* add carry */
    answer = ~sum;			/* truncate to 16 bits */
    return (answer);
}

/* To calculate UDP (same for TCP) checksum, a 12-byte pseudo-header (phdr) 
 * is used in addition to the UDP datagram.
 */
uint16_t udp_cksum (const struct iphdr *ip,
                    const struct udphdr *up, 
                    uint16_t udp_len)
{
    union phu {
        struct phdr {
            uint32_t src;
            uint32_t dst;
            uint8_t mbz;
            uint8_t proto;
            uint16_t len;
        } ph;
#define PSEUDO_HDR_LENGTH_IN_WORD 6
        uint16_t pa[PSEUDO_HDR_LENGTH_IN_WORD];
    } phu;
    const uint16_t *sp;

    /* pseudo-header.. */
    phu.ph.len = htons(udp_len);
    phu.ph.mbz = 0;
    phu.ph.proto = IPPROTO_UDP;
    memcpy(&phu.ph.src, &ip->saddr, sizeof(uint32_t));


    /* Original tcpdump code checks for possible ip options (source routing
     * for example) before setting phu.ph.dst.
     * 
     */
    if (ip->ihl == 5)
        memcpy(&phu.ph.dst, &ip->daddr, sizeof(u_int32_t));
    else
        return 0;
    
    sp = &phu.pa[0];
    return in_cksum((uint16_t *)up, udp_len,
                    sp[0]+sp[1]+sp[2]+sp[3]+sp[4]+sp[5]);
}



/* Perform the following sanity check on the potential STUN request frame:
 *  1. The frame has a complete IP header;
 *  2. The frame has a UDP header;
 *  3. UDP checksum is correct;
 *  4. The frame is long enough to hold the STUN header
 *
 * Return 0 if passed the check; -1 otherwise
 */
int sanity_check_packet_frame (const uint8_t *frame, const uint16_t length)
{
    uint16_t udp_csum;
    struct iphdr *ip_header = NULL;
    struct udphdr *udp_header = NULL;
    uint8_t iphdr_len = 0;  /* variable length depending on ->ihl */
    uint8_t udphdr_len = sizeof(struct udphdr);

    uint8_t *ptr = (uint8_t *)frame;
    int len = length;

    if (!frame) {
        return -1;
    }

    log_raw("passed in frame", ptr, len);

    /* Make sure that the frame is large enough to at least hold an 
     * ip header struct 
     */
    if (len < sizeof(struct iphdr)) {
        printf("Packet is too small to contain an ip header: %d\n", 
               sizeof(req_data));
        return -1;
    }
    ip_header = (struct iphdr *)ptr;

    if(ip_header->protocol != IPPROTO_UDP) {
        printf("Not a UDP packet: %d\n", ip_header->protocol);
        return -1;
    }

    /* Make sure it contains the whole ip header, which may be longer than
     * 20 bytes if it contains ip options.
     */
    iphdr_len = ip_header->ihl*4;
    if (len < iphdr_len) {
        printf("Packet (%d) is too small to contain the complete IP header. "
               "Whole IP header length is %d.\n",
               sizeof(req_data), ip_header->ihl*4);
        return -1;
    }
    ptr += iphdr_len;
    len -= iphdr_len;

    /* Check if we are large enough to contains a udp header */
    if (len < udphdr_len) {
        printf("Packet is too small to contain a udp header: %d\n", 
               sizeof(req_data));
        return -1;
    }

    udp_header = (struct udphdr *)ptr;

    /* Make sure the udp_header->len is right */
    if (ntohs(udp_header->len) != len) {
        printf("The length of %d in udp header is wrong: should be %d\n",
               ntohs(udp_header->len), len);
        return -1;
    }

    ptr += udphdr_len;
    len -= udphdr_len;

    /* Check if we have at least the fixed-length STUN hdr */
    if (len < sizeof(stun_header_t)) {
        printf("Packet is too small to contain a stun header: %d\n", len);
        return -1;
    }

    /* Check the udp checksum */
    if (udp_header->check) {
        udp_csum = udp_cksum(ip_header, udp_header, ntohs(udp_header->len));
        if (udp_csum) {
            printf("Invalid udp checksum: 0x%x\n", udp_header->check);
            return -1;
        }
    }

    return 0;
}


int main (void)
{
    uint8_t msg_buffer[1514];
    uint8_t *stun_msg_header = NULL;
    int stun_msg_len = 0;
    int ip_udp_hdr_len = 0;

    struct iphdr *ip_header = NULL;
    struct udphdr *udp_header = NULL;


    memcpy(msg_buffer, req_data, sizeof(req_data));

    if (sanity_check_packet_frame(msg_buffer, sizeof(req_data)) == -1) {
        printf("Sanity check failed\n");
        return -1;
    }

    printf("\nSanity checking passed\n");

    ip_header = (struct iphdr *)msg_buffer;
    udp_header = (struct udphdr*)(msg_buffer + ip_header->ihl*4);


    /* A stun message starts with the stun header */
    ip_udp_hdr_len = ip_header->ihl*4 + sizeof(struct udphdr);
    stun_msg_header = msg_buffer + ip_udp_hdr_len;
    stun_msg_len = sizeof(req_data) - ip_udp_hdr_len;


    printf("stun payload length: %d\n", stun_msg_len);
    log_raw("stun payload", stun_msg_header, stun_msg_len);

    log_raw("udp header", (uint8_t *)udp_header, sizeof(struct udphdr));
    log_raw("ip header", (uint8_t *)ip_header, ip_header->ihl*4);

    return 0;
}
