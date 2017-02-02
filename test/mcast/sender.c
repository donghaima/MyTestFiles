/* Modified from http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html */

/*
 * sender.c -- multicasts "hello, world!" to a multicast group once a second
 *
 * Antony Courtney,	25/11/94
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
     int fd, cnt;
     struct ip_mreq mreq;
     char *message="Hello World!";
     char msg[64];
     char *group = NULL;
     int port;

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

     /* set up destination address */
     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     //addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
     //addr.sin_port=htons(HELLO_PORT);
     addr.sin_addr.s_addr=inet_addr(group);
     addr.sin_port=htons(port);

     sprintf(msg, "%s to %s:%d", message, group, port);
     msg[strlen(msg)] = '\0';

     /* now just sendto() our destination! */
     while (1) {
          printf("To send message \"%s\" to %s:%d\n", msg, group, port);

	  if (sendto(fd, msg, strlen(msg)+1, 0, 
                     (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	       perror("sendto");
	       exit(1);
	  }
	  sleep(1);
     }
}
