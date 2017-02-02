/* sendmsg test program */

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
#include <errno.h>

/* Send data out from a specified interface, with a specified src IP address */
static int my_send (int fd, 
                    char *oif_name, struct in_addr srcip,
                    struct sockaddr_in *remote,
                    uint8_t *data, int len,
                    int flags)
{
    struct ifreq ifr;
    struct iovec iov = { data, len };

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, oif_name, strlen(oif_name));
    if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        fprintf(stderr, "unknown iface %s\n", oif_name);
        return(-1);
    }               

    struct {
        struct cmsghdr cm;
        struct in_pktinfo ipi;
    } cmsg = {
        .cm = {
            .cmsg_len = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo),
            .cmsg_level = SOL_IP,
            .cmsg_type = IP_PKTINFO,
        },
        .ipi = {
            .ipi_ifindex = ifr.ifr_ifindex,
            .ipi_spec_dst = srcip,
        },
    };

    struct msghdr m = {
        .msg_name = (void *)remote,
        .msg_namelen = sizeof(struct sockaddr_in),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = &cmsg,
        .msg_controllen = sizeof(cmsg),
        .msg_flags = 0,
    };

    return sendmsg(fd, &m, flags);
}



int main (int argc, char **argv)
{
    int fd, ret;
    struct sockaddr_in sa, da;
    struct in_addr srcip;
    uint8_t pkt_data[] = {'0','1','2','3','4','5',};

    if (argc < 6) {
        fprintf(stderr, 
                "Usage: %s <via if> <to ip> <to port> "
                "<src ip> <src port>\n", argv[0]);
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
    sa.sin_port = htons(atoi(argv[5]));
    //sa.sin_addr will be overwritten later anyway, no need to set it

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror ("bind:");
        exit(1);
    }

    /* Dest ip and port */
    memset(&da, 0, sizeof(sa));
    da.sin_family = AF_INET;
    da.sin_port = htons(atoi(argv[3]));
    da.sin_addr.s_addr = inet_addr(argv[2]);

    /* Packet src ip address */
    srcip.s_addr = inet_addr(argv[4]);

    while (1){
        sleep(1);
        ret = my_send(fd, argv[1], srcip, &da,
                      pkt_data, sizeof(pkt_data), 0);
        if (ret < 0)
            fprintf(stderr, "my_send() error: %d, %s\n",
                    errno, strerror(errno));
    }
}


