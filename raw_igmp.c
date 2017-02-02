/* Sniff IGMP packets with a raw IGMP socket: no packet filter */

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
    char buffer[100];

    memset(&saddr, 0, sizeof(struct sockaddr_in));

    /* Attach the filter to a raw IGMP socket */
    sd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if (sd==-1) {
        perror("socket");
        exit(0);
    }

    saddr_len = sizeof(saddr);
    while (n=recvfrom(sd, buffer, 100, 0,
                      (struct sockaddr*)&saddr,
                      &saddr_len)) {
        if (n==-1) {
            perror("recvfrom");
            close(sd);
            exit(0);
        }

        printf("raw_igmp.c: received from %s:%d: %d bytes\n", 
               inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port), n);
        
        dump_raw(buffer, n);
    }

    close(sd);
}
