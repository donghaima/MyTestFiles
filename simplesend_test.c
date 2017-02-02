/* Use sendto to send a packet */

#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main (int argc, char **argv)
{
    int fd, ret;
    struct sockaddr_in sa, da;
    struct in_addr srcip;
    uint8_t pkt_data[] = {'0','1','2','3','4','5',};

    if (argc < 5) {
        fprintf(stderr, 
                "Usage: %s <to ip> <to port> <src ip> <src port>\n", argv[0]);
        exit(1);
    }
    
    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror ("socket:");
        exit(1);
    }

    /* Setup the packet source port */
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[4]));
    sa.sin_addr.s_addr = inet_addr(argv[3]);

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror ("bind:");
        exit(1);
    }

    /* Dest ip and port */
    memset(&da, 0, sizeof(sa));
    da.sin_family = AF_INET;
    da.sin_port = htons(atoi(argv[2]));
    da.sin_addr.s_addr = inet_addr(argv[1]);

    while (1){
        sleep(1);
        ret = sendto(fd, pkt_data, sizeof(pkt_data), 0, 
                     (struct sockaddr *)&da, sizeof(da));
        if (ret < 0) {
            perror("sendto:");
        }
    }
}


