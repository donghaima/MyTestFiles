
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/filter.h>
#include <sys/ioctl.h>

int main() {

  int sd, saddr_len, n;
  int uno=1;
  struct sockaddr_in addr, saddr;
  char buffer[1000];

  struct sock_filter BPF_code[]= {
    /* Get first two bytes of packet payload
     * (skipping 8 bytes of UDP header data)  */
    { 0x28, 0, 0,  0x00000008 },
    /* Compare them with 'lj' */
    { 0x15, 0, 1,  0x00006c6a },
    /* Accept packet if match */
    { 0x06, 0, 0, 0x100 },
    /* Reject packet otherwise */
    { 0x06, 0, 0, 0x00 }
  };
  struct sock_fprog Filter;
 
  Filter.len = 4;
  Filter.filter = BPF_code;

  bzero(&addr, sizeof(struct sockaddr_in));
  bzero(&saddr, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
//  addr.sin_port = htons(6600);
  //addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  sd = socket(PF_INET, SOCK_DGRAM, 0);
  if (sd==-1) {
      perror("socket");
      exit(0);
    }
 
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &uno,
	     sizeof(uno));
  if ((setsockopt(sd, SOL_SOCKET, SO_ATTACH_FILTER,
		  &Filter, sizeof(Filter)))==-1) {
    printf("errno = %d\n", errno);
    perror("setsockopt()");
    close(sd);
    exit(1);
  }

  bind(sd, (struct sockaddr *)&addr, sizeof(addr));

  saddr_len = sizeof(saddr);
  while (n=recvfrom(sd, buffer, 1000, 0,
		    (struct sockaddr*)&saddr,
		    &saddr_len)) {
    if (n==-1) {
      perror("recvfrom");
      close(sd);
      exit(0);
    }
    buffer[n]=0;
    printf("Received from %s (ports %d,%d) %d bytes\n"
	   "Data: <%s>\n", inet_ntoa(saddr.sin_addr),
	   ntohs(saddr.sin_port),
	   ntohs(addr.sin_port), n, buffer);
  }

  close(sd);
}
