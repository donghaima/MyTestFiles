/* Modified from http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html */

/*
 * listener.c -- joins a multicast group and echoes all data it receives from
 *		the group to its stdout...
 *
 * Antony Courtney,	25/11/94
 * Modified by: Frédéric Bastien (25/03/04)
 * to compile without warning and work correctly
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "def.h"


main(int argc, char *argv[])
{
     struct sockaddr_in addr;
     int fd, nbytes,addrlen;
     struct ip_mreq mreq;
     char msgbuf[MSGBUFSIZE];
     char *group = NULL;
     int port;

     u_int yes=1;            /*** MODIFICATION TO ORIGINAL */

     if (argc == 3) {
         group = argv[1];
         port = atoi(argv[2]);
     } else {
         group = HELLO_GROUP;
         port = HELLO_PORT;
     }
         

     /* create what looks like an ordinary UDP socket */
     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("socket");
	  exit(1);
     }

/**** MODIFICATION TO ORIGINAL */
    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
       perror("Reusing ADDR failed");
       exit(1);
       }
/*** END OF MODIFICATION TO ORIGINAL */

     /* set up destination address */
     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */

     /* DoMa: Need to bind to the specific address to filter out other mcast traffic */ 
     addr.sin_addr.s_addr=inet_addr(group);

     //addr.sin_port=htons(HELLO_PORT);
     addr.sin_port=htons(port);
     
     /* bind to receive address */
     if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	  perror("bind");
	  exit(1);
     }
     
     /* use setsockopt() to request that the kernel join a multicast group */
     //mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);

     mreq.imr_multiaddr.s_addr=inet_addr(group);
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);

     if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
	  perror("setsockopt");
	  exit(1);
     }

     printf("Joined group %s and bound to %s:%d...\n", group, group, port);

     /* now just enter a read-print loop */
     while (1) {
	  addrlen=sizeof(addr);
	  if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
			       (struct sockaddr *) &addr,&addrlen)) < 0) {
	       perror("recvfrom");
	       exit(1);
	  }
	  printf("nbytes=%d, msgbuf=%s\n", nbytes, msgbuf);
     }
}
