
/*
 * igmp_join.c -- Join a multicast group
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <net/if.h>
#include <errno.h>

#define DEFAULTPORT  50000
#define DEFAULTGROUP "235.0.0.1"

#define MSGBUFSIZE   15

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, nbytes,addrlen;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE] = {0};
    char *group = NULL;
    char *if_name = NULL;
    int port = DEFAULTPORT;

    u_int yes = 1;

    if (argc == 3) {
        group = argv[1];
        if_name = argv[2];
    } else {
        group = DEFAULTGROUP;
        if_name = "lo";
    }
         
    /* Create an ordinary UDP socket */
    if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return -1;
    }

    /* Allow multiple sockets to use the same PORT number */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        return -1;
    }

    /* Specify the interface where this socke is bound */
    struct ifreq   ifreq;
    struct in_addr if_addr;
    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, if_name, IFNAMSIZ);
    ifreq.ifr_addr.sa_family = AF_INET;
    
    if (ioctl(fd, SIOCGIFADDR, &ifreq) < 0) {
        printf("ioctl() failed\n");
        return(-1);
    }
        
    if_addr.s_addr = 
        ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr;


    /* set up destination address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //addr.sin_addr.s_addr = if_addr.s_addr;

    /* Need to bind to the specific address to filter out other 
       mcast traffic */ 
    addr.sin_addr.s_addr = inet_addr(group);
    addr.sin_port = htons(port);
     
    /* bind to receive address */
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
     
    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    //mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_interface.s_addr = if_addr.s_addr;

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) 
        < 0) {
        perror("setsockopt");
        return -1;
    }

    printf("Joined group %s and bound to %s:%d...\n", group, group, port);

    /* now just enter a read-print loop */
    while (1) {
        addrlen = sizeof(addr);
        if ((nbytes = recvfrom(fd,msgbuf,MSGBUFSIZE, MSG_TRUNC,
			       (struct sockaddr *) &addr,&addrlen)) < 0) {
            perror("recvfrom");
            return -1;
        }
        
        // Force '\0' at the end of the msgbug[] for printf()
        msgbuf[MSGBUFSIZE] = '\0';
        printf("nbytes=%d, msgbuf=%s\n", nbytes, msgbuf);
    }

    return 0;
}
