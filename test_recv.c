/*
 * Compile with:
 *  gcc -g -static -I/users/doma/local/include -o test_recv test_recv.c -L/users/doma/local/lib -levent
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <event.h>

#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


/* socket read */
void socket_read(int fd, short event, void *arg)
{
    char buf[1500] = {0};
    struct event *ev = arg;

    struct sockaddr_in from;
    socklen_t  len = sizeof(from);

    /* Reschedule this event */
    event_add(ev, NULL);

    fprintf(stderr, "socket_read called with fd: %d, event: %d, arg: %p\n",
            fd, event, arg);

    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, 
                         (struct sockaddr *)&from, &len);
    if (n < 0) {
        /* recvfrom() error */
        perror("recvfrom()");
        return;
    }
    
    buf[len] = '\0';

    fprintf(stdout, "Read %d bytes from 0x%x: %s\n", n, &from, buf);
}


/* Setup a listening UDP socket for unicast or multicast */
#define MCAST    1
#define UCAST    2
int setup_socket(int type, in_addr_t addr, int port)
{
    int on = 1, ret;
    struct sockaddr_in	servaddr;
    struct ip_mreq mreq;

    /* Create a rudimentary socket */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("open");
        return(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = htons(port);

    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        perror("setsockopt");
        return ret;
    }

    ret = bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind");
        return ret;
    }

    if (MCAST == type) {
        /* use setsockopt() to request that the kernel join a multicast group */
        mreq.imr_multiaddr.s_addr = addr;
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            perror("mcast setsockopt");
            return(-1);
        }
    }

    return fd;
}


/* Test routine to add two socket events (one for multicast and one for unicast)
 and one timer event to the dispatcher. */
int main (int argc, char **argv)
{
    struct event evsocket, timeout, mcast_evsocket;

    int	sockfd, mcast_sockfd;
    int port = 6600;

    /* Setup a unicast UDP listening socket */
    //sockfd = setup_socket(UCAST, htonl(INADDR_ANY), port);
    sockfd = setup_socket(UCAST, inet_addr("127.0.0.1"), port);
    if (sockfd == -1) {
        perror("setup a UDP listening socket");
        return -1;
    }

    fprintf(stderr, "%s: Socket %d ready to receive packets on port %d...\n", 
            argv[0], sockfd, port);

    /* Setup a multicast listening socket */
    mcast_sockfd = setup_socket(MCAST, inet_addr("224.10.10.10"), port);
    if (mcast_sockfd == -1) {
        perror("setup a mcast listening socket");
        return -1;
    }

    fprintf(stderr, "Mcast socket %d ready to receive packets from group %s, port %d...\n", 
            mcast_sockfd, "224.10.10.10", port);

    /* Initalize the event library */
    event_init();

    /* Initalize two socket events: use the same receiving handler for now */
    event_set(&evsocket, sockfd, EV_READ, socket_read, &evsocket);
    event_set(&mcast_evsocket, mcast_sockfd, EV_READ, socket_read, &mcast_evsocket);

    /* Add it to the active events, without a timeout */
    event_add(&evsocket, NULL);
    event_add(&mcast_evsocket, NULL);


    /* Dispatcher */
    event_dispatch();

    return (0);
}

