
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
//#include <arpa/inet.h>
#include <linux/filter.h>
#include <sys/ioctl.h>


void dump_raw (unsigned char *buf, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (i>0 && (i & 15) == 0) {
            printf("\n");
        } else if (i>0 && (i & 7) == 0)
            printf(" ");

        printf(" %02x", buf [i]);
    }
    printf("\n");
}


int main() {

  int sd, saddr_len, n;
  int uno=1;
  //struct sockaddr_in addr, saddr;
  struct sockaddr_in saddr;
  char buffer[1000];

  /* Be careful about the packet offset when using a raw socket */
  struct sock_filter BPF_code[]= {
      /* Get first two bytes of packet payload
       * (skipping 28 bytes of IP/UDP header data)  */
      { 0x28, 0, 0,  0x0000001c },
      //* (skipping 8 bytes of UDP header data)  */
      //{ 0x28, 0, 0,  0x00000008 },
      /* Compare them with 'lj' */
      { 0x15, 0, 1,  0x00006c6a },
      /* Accept packet if match */
      { 0x06, 0, 0, 0x100 },
      /* Reject packet otherwise */
      { 0x06, 0, 0, 0x00 }
  };

  struct sock_fprog Filter;
 
  Filter.len = sizeof(BPF_code)/sizeof(BPF_code[0]);
  Filter.filter = BPF_code;

  bzero(&saddr, sizeof(struct sockaddr_in));

  /* Attach the filter to a raw socket */
  sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
  if (sd==-1) {
      perror("socket");
      exit(0);
    }
 
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &uno, sizeof(uno));

  if ((setsockopt(sd, SOL_SOCKET, SO_ATTACH_FILTER,
		  &Filter, sizeof(Filter))) == -1) {
    printf("errno = %d\n", errno);
    perror("setsockopt()");
    close(sd);
    exit(1);
  }

  //bind(sd, (struct sockaddr *)&addr, sizeof(addr));

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
    printf("Received from %s:%d: %d bytes\n", 
	   inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port), n);

    dump_raw(buffer, n);

  }

  close(sd);
}
