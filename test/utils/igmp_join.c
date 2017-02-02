
/*
 * igmp_join.c -- Join a multicast group
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define DEFAULTPORT  12345
#define DEFAULTGROUP "225.0.0.38"
#define MSGBUFSIZE   1500

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, nbytes,addrlen;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];
    char *group = NULL;
    int port;

    u_int yes = 1;

    if (argc == 3) {
        group = argv[1];
        port = atoi(argv[2]);
    } else {
        group = DEFAULTGROUP;
        port = DEFAULTPORT;
    }
         
    /* Create an ordinary UDP socket */
    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return -1;
    }

    /* Allow multiple sockets to use the same PORT number */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        return -1;
    }

    /* set up destination address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Need to bind to the specific address to filter out other 
       mcast traffic */ 
    addr.sin_addr.s_addr=inet_addr(group);
    addr.sin_port=htons(port);
     
    /* bind to receive address */
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
     
    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr=inet_addr(group);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) 
        < 0) {
        perror("setsockopt");
        return -1;
    }

    printf("Joined group %s and bound to %s:%d...\n", group, group, port);

    /* now just enter a read-print loop */
    while (1) {
        addrlen = sizeof(addr);
        if ((nbytes = recvfrom(fd,msgbuf,MSGBUFSIZE,0,
			       (struct sockaddr *) &addr,&addrlen)) < 0) {
            perror("recvfrom");
            return -1;
        }
        printf("nbytes=%d, msgbuf=%s\n", nbytes, msgbuf);
    }

    return 0;
}
