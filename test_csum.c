#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>

int main (void)
{
    int sd;

    if ((sd = socket(PF_PACKET, SOCK_RAW,
                     htons((short)ETH_P_ALL))) < 0) {

    }

    int val;

    val = 1;
    if (setsockopt (sd, SOL_PACKET, PACKET_AUXDATA, &val,
                    sizeof val) < 0) {
        if (errno != ENOPROTOOPT)
            log_fatal ("Failed to set auxiliary packet data: %m");
    }
    

    int nocsum = 0;
    char msgname[32];
    unsigned char ibuf [1536];
    unsigned char cmsgbuf[CMSG_LEN(sizeof(struct tpacket_auxdata))];
    struct iovec iov = {
        .iov_base = ibuf,
        .iov_len = sizeof ibuf,
    };
    struct msghdr msg = {
        .msg_name = msgname;
        .msg_namelen = 32;
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = cmsgbuf,
        .msg_controllen = sizeof(cmsgbuf),
    };
    struct cmsghdr *cmsg;
    
    length = recvmsg (sd, &msg, 0);
    if (length <= 0)
        return length;
    
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_PACKET &&
            cmsg->cmsg_type == PACKET_AUXDATA) {
            struct tpacket_auxdata *aux = (void *)CMSG_DATA(cmsg);
            nocsum = aux->tp_status & TP_STATUS_CSUMNOTREADY;
        }
    }

    return 1;
}
