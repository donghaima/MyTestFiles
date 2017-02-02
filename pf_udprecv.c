
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

int main() {

  int sd, saddr_len, n;
  int uno=1;
  //struct sockaddr_in addr, saddr;
  struct sockaddr_in saddr;
  char buffer[1000];

#if 0
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
#endif

#if 0
  [root@pissarro test]# tcpdump -d udp and port 6600
      (000) ldh      [12]
      (001) jeq      #0x86dd          jt 2    jf 8
      (002) ldb      [20]
      (003) jeq      #0x11            jt 4    jf 19
      (004) ldh      [54]
      (005) jeq      #0x19c8          jt 18   jf 6
      (006) ldh      [56]
      (007) jeq      #0x19c8          jt 18   jf 19
      (008) jeq      #0x800           jt 9    jf 19
      (009) ldb      [23]
      (010) jeq      #0x11            jt 11   jf 19
      (011) ldh      [20]
      (012) jset     #0x1fff          jt 19   jf 13
      (013) ldxb     4*([14]&0xf)
      (014) ldh      [x + 14]
      (015) jeq      #0x19c8          jt 18   jf 16
      (016) ldh      [x + 16]
      (017) jeq      #0x19c8          jt 18   jf 19
      (018) ret      #96
      (019) ret      #0
#endif

   struct sock_filter BPF_code[]= {
      { 0x28, 0, 0, 0x0000000c },
      { 0x15, 0, 6, 0x000086dd },
      { 0x30, 0, 0, 0x00000014 },
      { 0x15, 0, 15, 0x00000011 },
      { 0x28, 0, 0, 0x00000036 },
      { 0x15, 12, 0, 0x000019c8 },
      { 0x28, 0, 0, 0x00000038 },
      { 0x15, 10, 11, 0x000019c8 },
      { 0x15, 0, 10, 0x00000800 },
      { 0x30, 0, 0, 0x00000017 },
      { 0x15, 0, 8, 0x00000011 },
      { 0x28, 0, 0, 0x00000014 },
      { 0x45, 6, 0, 0x00001fff },
      { 0xb1, 0, 0, 0x0000000e },
      { 0x48, 0, 0, 0x0000000e },
      { 0x15, 2, 0, 0x000019c8 },
      { 0x48, 0, 0, 0x00000010 },
      { 0x15, 0, 1, 0x000019c8 },
      { 0x6, 0, 0, 0x00000060 },
      { 0x6, 0, 0, 0x00000000 },
  };


  struct sock_fprog Filter;
 
  Filter.len = sizeof(BPF_code)/sizeof(BPF_code[0]);
  Filter.filter = BPF_code;

  bzero(&saddr, sizeof(struct sockaddr_in));
  //bzero(&addr, sizeof(struct sockaddr_in));
  //addr.sin_family = AF_INET;
  //addr.sin_port = htons(6600);
  //addr.sin_addr.s_addr = INADDR_ANY;

  //sd = socket(PF_PACKET, SOCK_DGRAM, 0);
  sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
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
    printf("Received from %s:%d: %d bytes\n"
	   "Data: <%s>\n", inet_ntoa(saddr.sin_addr),
	   ntohs(saddr.sin_port), n, buffer);
  }

  close(sd);
}
