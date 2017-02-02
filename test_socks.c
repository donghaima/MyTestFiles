/*
 * Compile with:
 *  gcc -g -static -I/users/doma/local/include -o test test.c -L/users/doma/local/lib -levent
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <event.h>

#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TIMER_INTERVAL 20

int lasttime;

/* timer handler: get called every 2 seconds  */
void timeout_cb(int fd, short event, void *arg)
{
    struct timeval tv;
    struct event *timeout = arg;
    int newtime = time(NULL);
    
    //printf("%s: called at %d: %d\n", __func__, newtime,
    //        newtime - lasttime);
    fprintf(stdout, ".");
    fflush(stdout);

    lasttime = newtime;
    
    timerclear(&tv);
    tv.tv_sec = TIMER_INTERVAL;
    event_add(timeout, &tv);
}

/* socket read */
void socket_read(int fd, short event, void *arg)
{
    char buf[1500] = {0};
    struct event *ev = arg;

    struct sockaddr_in from;
    socklen_t  len = sizeof(from);

    /* Reschedule this event */
    //event_add(ev, NULL);

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

    ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        perror("fcntl: O_NONBLOCK");
        return ret;
    }

    ret = bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind");
        return ret;
    }

    if (MCAST == type) {
        /* join a multicast group */
        mreq.imr_multiaddr.s_addr = addr;
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) 
            < 0) {
            perror("mcast setsockopt");
            return(-1);
        }
    }

    return fd;
}


#define TOTAL_CHANS  500
#define PORT_BASE    20000

/* Test routine to 
   and one timer event to the dispatcher. */
int main (int argc, char **argv)
{
    struct event evsocket[TOTAL_CHANS], timeout;

    int	sockfd[TOTAL_CHANS], sockfd2[TOTAL_CHANS];
    int port;
    int i;

    /* Initalize the event library */
    event_init();

    for (i=0; i<TOTAL_CHANS; i++) {
        port = PORT_BASE + i;

        /* Setup a unicast UDP listening socket */
        sockfd[i] = setup_socket(UCAST, inet_addr("1.1.1.1"), port);
        if (sockfd[i] == -1) {
            perror("setup a UDP listening socket");
            return -1;
        }

        fprintf(stderr, "%d: Socket %d ready to receive packets on port %d\n",
                i, sockfd[i], port);

        /* Setup another unicast UDP socket, bind to the same port */
        //sockfd2[i] = setup_socket(UCAST, htonl(INADDR_ANY), port);
        sockfd2[i] = setup_socket(UCAST, inet_addr("1.1.1.1"), port);
        if (sockfd2[i] == -1) {
            perror("setup another UDP socket");
            return -1;
        }

        fprintf(stderr, "%d: Socket %d ready to receive packets on port %d\n",
                i, sockfd2[i], port);


        /* Initalize socket events: use the same receiving handler for now */
        //event_set(&evsocket[i], sockfd[i], EV_READ, socket_read, 
        //          &evsocket[i]);
        event_set(&evsocket[i], sockfd2[i], EV_READ, socket_read, 
                  &evsocket[i]);

        /* Add it to the active events */
        event_add(&evsocket[i], NULL);
    }

   /* Initalize one timer event */
    struct timeval tv;
    evtimer_set(&timeout, timeout_cb, &timeout);

    timerclear(&tv);
    tv.tv_sec = TIMER_INTERVAL;
    event_add(&timeout, &tv);
    
    lasttime = time(NULL);

    /* Dispatcher */
    event_dispatch();

    return (0);
}

