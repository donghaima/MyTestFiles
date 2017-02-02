#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>


int create_udp_recv_socket (in_addr_t addr, in_port_t port, in_addr_t if_addr)
{
    int on = 1, ret;
    struct sockaddr_in	servaddr;
    struct ip_mreq mreq;

    printf("Passed in(network order) - addr: %x:%d, if_addr:%x\n", 
           ntohl(addr), ntohs(port), ntohl(if_addr));

    /* Create a rudimentary socket */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("open");
        return(-1);
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        perror("setsockopt");
        return ret;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    if (IN_MULTICAST(ntohl(addr))) {
        /* request the kernel to join a multicast group */
        mreq.imr_multiaddr.s_addr = addr;
        mreq.imr_interface.s_addr = if_addr;

        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
                       &mreq, sizeof(mreq)) < 0) {
            perror("mcast setsockopt");
            return(-1);
        }
    } 


    ret = bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind");
        return ret;
    }

    printf("Created a listening socket on %s:%d\n", 
           inet_ntoa(servaddr.sin_addr), htons(servaddr.sin_port));

    return fd;
}



int main(void)
{
    int sock;
    const char *addr = "7.1.2.3";
    char cmd[32];
    in_port_t port = htons(5432);
    int first_time = 1;
    int ret;

    while (1) {
        sock = create_udp_recv_socket(inet_addr(addr), port, INADDR_NONE);

        if (sock < 0) {
            fprintf(stderr, "socket(%d):%m\n", errno);

            /* Add addr to lo */
            snprintf(cmd, 32, "ip addr add %s/32 dev lo", addr);
            ret = system(cmd);
            printf("Run system command: %s; ret=%d; ip addr show dev lo:\n", 
                   cmd, ret);
            system("ip addr show dev lo");
            
            /* Try to add the address again, expect to see error */
            ret = system(cmd);
            printf("Run again: %s; ret=%d\n", cmd, ret);

        } else {
            printf("Created a UDP listening socket: %d; %s:%d\n", 
                   sock, addr, ntohs(port));

            if (first_time) {
                printf("Remove address %s from dev lo; and try to bind socket "
                       "again\n", addr);
                snprintf(cmd, 32, "ip addr del %s/32 dev lo", addr);
                assert(system(cmd) == 0);
                printf("Run system command: %s; ip addr show dev lo:\n", cmd);
                system("ip addr show dev lo");

                first_time = 0;
            } else {
                printf("We are done. ip addr show dev lo:\n");
                system("ip addr show dev lo");
                break;
            }
        }
        printf("\n\n");
        sleep(1);
    }

    sleep (10);
    return (0);
}
