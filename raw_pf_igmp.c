/* Sniff IGMP packets from a raw IP socket: with IGMP packet filter */

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



#if COMMENTS
[root@pissarro ~]# tcpdump -d igmp
(000) ldh      [12]
(001) jeq      #0x800           jt 2    jf 5
(002) ldb      [23]
(003) jeq      #0x2             jt 4    jf 5
(004) ret      #96
(005) ret      #0

[root@pissarro ~]# tcpdump -dd igmp
{ 0x28, 0, 0, 0x0000000c },
{ 0x15, 0, 3, 0x00000800 },
{ 0x30, 0, 0, 0x00000017 },
{ 0x15, 0, 1, 0x00000002 },
{ 0x6, 0, 0, 0x00000060 },
{ 0x6, 0, 0, 0x00000000 },
#endif


int main() 
{
    int sd, saddr_len, n;
    int uno=1;
    //struct sockaddr_in addr, saddr;
    struct sockaddr_in saddr;
    char buffer[100];
    unsigned char *iphead, *ethhead;

    struct sock_filter BPF_code[]= { 
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 0, 3, 0x00000800 },
        { 0x30, 0, 0, 0x00000017 },
        { 0x15, 0, 1, 0x00000002 },
        { 0x6, 0, 0, 0x00000060 },
        { 0x6, 0, 0, 0x00000000 },
    };

    struct sock_fprog Filter;
 
    Filter.len = sizeof(BPF_code)/sizeof(BPF_code[0]);
    Filter.filter = BPF_code;

    memset(&saddr, 0, sizeof(struct sockaddr_in));

    /* Attach the filter to a raw IP socket */
    sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
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
    while (n=recvfrom(sd, buffer, 100, 0, NULL, NULL)) {

        printf("%d bytes read\n",n);

        /* Check to see if the packet contains at least 
         * complete Ethernet (14), IP (20) and TCP/UDP 
         * (8) headers.
         */
        if (n<42) {
            perror("recvfrom():");
            printf("Incomplete packet (errno is %d)\n",
                   errno);
            close(sd);
            exit(0);
        }


        ethhead = buffer;
        printf("Source MAC address: "
               "%02x:%02x:%02x:%02x:%02x:%02x\n",
               ethhead[0],ethhead[1],ethhead[2],
               ethhead[3],ethhead[4],ethhead[5]);
        printf("Destination MAC address: "
               "%02x:%02x:%02x:%02x:%02x:%02x\n",
               ethhead[6],ethhead[7],ethhead[8],
               ethhead[9],ethhead[10],ethhead[11]);

        iphead = buffer+14; /* Skip Ethernet  header */
        if (*iphead==0x46) { /* Double check for IPv4 
                              * and no options present */
            printf("Source host %d.%d.%d.%d\n",
                   iphead[12],iphead[13],
                   iphead[14],iphead[15]);
            printf("Dest host %d.%d.%d.%d\n",
                   iphead[16],iphead[17],
                   iphead[18],iphead[19]);
            printf("Source,Dest ports %d,%d\n",
                   (iphead[20]<<8)+iphead[21],
                   (iphead[22]<<8)+iphead[23]);
            printf("Layer-4 protocol %d\n",iphead[9]);
        }

        dump_raw(buffer, n);
    }

    close(sd);
}
