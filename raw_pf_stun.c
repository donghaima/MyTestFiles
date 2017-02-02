
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
//#include <arpa/inet.h>
#include <linux/filter.h>
#include <sys/ioctl.h>


void dump_raw (unsigned char *buf, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (i>0 && (i & 15) == 0) {
            printf("\n");
        } else if (i>0 && (i & 7) == 0)
            printf(" ");

        printf(" %02x", buf [i]);
    }
    printf("\n");
}


int main() 
{
    int sd, saddr_len, n;
    int uno=1;
    //struct sockaddr_in addr, saddr;
    struct sockaddr_in saddr;
    char buffer[1000];

#if 0  // A UDP packet starts with 'lj' 
    /* Be careful about the packet offset when using a raw socket */
    struct sock_filter BPF_code[]= {
        /* Get first two bytes of packet payload
         * (skipping 28 bytes of IP/UDP header data)  */
        { 0x28, 0, 0,  0x0000001c },
        /* Compare them with 'lj' */
        { 0x15, 0, 1,  0x00006c6a },
        /* Accept packet if match */
        { 0x06, 0, 0, 0x100 },
        /* Reject packet otherwise */
        { 0x06, 0, 0, 0x00 }
    };
#endif


#if 0
# tcpdump -s 256 -d "(not ip multicast) && (udp[8]=0) && (udp[10:2]&0x3=0) && (udp[12:4]=0x2112a442)"
    (000) ldh      [12]
    (001) jeq      #0x800           jt 2    jf 16
    (002) ldb      [30] -> [16]
    (003) jge      #0xe0            jt 16   jf 4
    (004) ldb      [23] -> [9]
    (005) jeq      #0x11            jt 6    jf 16
    (006) ldh      [20] -> [6]
    (007) jset     #0x1fff          jt 16   jf 8
    (008) ldxb     4*([14]&0xf)
    (009) ldb      [x + 22]
    (010) jeq      #0x0             jt 11   jf 16
    (011) ldh      [x + 24]
    (012) jset     #0x3             jt 16   jf 13
    (013) ld       [x + 26]
    (014) jeq      #0x2112a442      jt 15   jf 16
    (015) ret      #256
    (016) ret      #0

# tcpdump -s 256 -dd "(not ip multicast) && (udp[8]=0) && (udp[10:2]&0x3=0) && (udp[12:4]=0x2112a442)"
    { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 14, 0x00000800 },
    { 0x30, 0, 0, 0x0000001e },
    { 0x35, 12, 0, 0x000000e0 },
    { 0x30, 0, 0, 0x00000017 },
    { 0x15, 0, 10, 0x00000011 },
    { 0x28, 0, 0, 0x00000014 },
    { 0x45, 8, 0, 0x00001fff },
    { 0xb1, 0, 0, 0x0000000e },
    { 0x50, 0, 0, 0x00000016 },
    { 0x15, 0, 5, 0x00000000 },
    { 0x48, 0, 0, 0x00000018 },
    { 0x45, 3, 0, 0x00000003 },
    { 0x40, 0, 0, 0x0000001a },
    { 0x15, 0, 1, 0x2112a442 },
    { 0x6, 0, 0, 0x00000100 },
    { 0x6, 0, 0, 0x00000000 },


    /* Modify the filter: no need to check if it is IP or UDP here as we
     * are receiving via a raw UDP socket.  Also need to subtract 
     * 14-byte ethernet header.
     */
    (000) ldb      [16]
    (001) jge      #0xe0            jt 12   jf 2
    (002) ldh      [6]
    (003) jset     #0x1fff          jt 12   jf 4
    (004) ldxb     4*([0]&0xf)
    (005) ldb      [x + 8]
    (006) jeq      #0x0             jt 7   jf 12
    (007) ldh      [x + 10]
    (008) jset     #0x3             jt 12   jf 9
    (009) ld       [x + 12]
    (010) jeq      #0x2112a442      jt 11   jf 12
    (011) ret      #256
    (012) ret      #0
#endif

    struct sock_filter BPF_code[]= { 
        { 0x30, 0, 0, 0x00000010 },
        { 0x35, 10, 0, 0x000000e0 },
        { 0x28, 0, 0, 0x00000006 },
        { 0x45, 8, 0, 0x00001fff },
        { 0xb1, 0, 0, 0x00000000 },
        { 0x50, 0, 0, 0x00000008 },
        { 0x15, 0, 5, 0x00000000 },
        { 0x48, 0, 0, 0x0000000a },
        { 0x45, 3, 0, 0x00000003 },
        { 0x40, 0, 0, 0x0000000c },
        { 0x15, 0, 1, 0x2112a442 },
        { 0x6, 0, 0, 0x00000100 },
        { 0x6, 0, 0, 0x00000000 },
    };

    struct sock_fprog Filter;
 
    Filter.len = sizeof(BPF_code)/sizeof(BPF_code[0]);
    Filter.filter = BPF_code;

    memset(&saddr, 0, sizeof(struct sockaddr_in));

    /* Attach the filter to a raw UDP socket */
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sd==-1) {
        perror("socket");
        exit(0);
    }

    if ((setsockopt(sd, SOL_SOCKET, SO_ATTACH_FILTER,
                    &Filter, sizeof(Filter))) == -1) {
        printf("errno = %d\n", errno);
        perror("setsockopt()");
        close(sd);
        exit(1);
    }

    saddr_len = sizeof(saddr);
    while (n=recvfrom(sd, buffer, 1000, 0,
                      (struct sockaddr*)&saddr,
                      &saddr_len)) {
        if (n==-1) {
            perror("recvfrom");
            close(sd);
            exit(0);
        }

        printf("Received from %s:%d: %d bytes\n", 
               inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port), n);
        
        dump_raw(buffer, n);
    }

    close(sd);
}
