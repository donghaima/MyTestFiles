
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>

int main (int argc, char *argv[])
{
    int sock_fd, retval;
    int rtcp_dscp = 63;
    socklen_t optlen = sizeof(retval);

    if (argc > 1) {
        rtcp_dscp = atoi(argv[1]);
    }

    int dscp = (int)(rtcp_dscp << 2);

    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return (-1);
    }

    //if (setsockopt(sock_fd, SOL_IP, IP_TOS, &dscp, sizeof(dscp)) < 0) {
    if (setsockopt(sock_fd, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp))) {
        perror("setsockopt");
        return (-1);
    }

    printf("In %s: set IP_TOS to %d with setsockopt()\n", argv[0], dscp);

    if (getsockopt(sock_fd,  SOL_IP, IP_TOS, &retval, &optlen) < 0) {
        perror("getsockopt");
        return (-1);
    }

    printf("In %s: get IP_TOS %d with getsockopt()\n", argv[0], retval);

    sleep (10);

    return 0;

}

