#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
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



int main(int argc, char **argv) {
    int sock, n, i;
  char buffer[2048];
  unsigned char *iphead, *ethhead, *udphead;
  int udp_plen;

  struct sock_fprog Filter; 

#if 0
[root@pissarro test]# tcpdump -d not ip multicast and udp and port 6600
(000) ldh      [12]
(001) jeq      #0x800           jt 2    jf 13
(002) ldb      [30]
(003) jge      #0xe0            jt 21   jf 4
(004) ldb      [23]
(005) jeq      #0x11            jt 6    jf 21
(006) ldh      [20]
(007) jset     #0x1fff          jt 21   jf 8
(008) ldxb     4*([14]&0xf)
(009) ldh      [x + 14]
(010) jeq      #0x19c8          jt 20   jf 11
(011) ldh      [x + 16]
(012) jeq      #0x19c8          jt 20   jf 21
(013) jeq      #0x86dd          jt 14   jf 21
(014) ldb      [20]
(015) jeq      #0x11            jt 16   jf 21
(016) ldh      [54]
(017) jeq      #0x19c8          jt 20   jf 18
(018) ldh      [56]
(019) jeq      #0x19c8          jt 20   jf 21
(020) ret      #96
(021) ret      #0
#endif

  struct sock_filter BPF_code[]= { 
    { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 11, 0x00000800 },
    { 0x30, 0, 0, 0x0000001e },
    { 0x35, 17, 0, 0x000000e0 },
    { 0x30, 0, 0, 0x00000017 },
    { 0x15, 0, 15, 0x00000011 },
    { 0x28, 0, 0, 0x00000014 },
    { 0x45, 13, 0, 0x00001fff },
    { 0xb1, 0, 0, 0x0000000e },
    { 0x48, 0, 0, 0x0000000e },
    { 0x15, 9, 0, 0x000019c8 },
    { 0x48, 0, 0, 0x00000010 },
    { 0x15, 7, 8, 0x000019c8 },
    { 0x15, 0, 7, 0x000086dd },
    { 0x30, 0, 0, 0x00000014 },
    { 0x15, 0, 5, 0x00000011 },
    { 0x28, 0, 0, 0x00000036 },
    { 0x15, 2, 0, 0x000019c8 },
    { 0x28, 0, 0, 0x00000038 },
    { 0x15, 0, 1, 0x000019c8 },
    //{ 0x6, 0, 0, 0x00000060 },
    { 0x6, 0, 0, 0x00000100 },
    { 0x6, 0, 0, 0x00000000 },
  };

  Filter.len = sizeof(BPF_code)/sizeof(BPF_code[0]);
  Filter.filter = BPF_code;

  if ( (sock=socket(PF_PACKET, SOCK_RAW, 
                    htons(ETH_P_IP)))<0) {
    perror("socket");
    exit(1);
  }

  /* Attach the filter to the socket */
  if(setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, 
                &Filter, sizeof(Filter))<0){
    perror("setsockopt");
    close(sock);
    exit(1);
  }
 
  while (1) {
    printf("----------\n");
    n = recvfrom(sock,buffer,2048,0,NULL,NULL);
    printf("%d bytes read\n",n);

    /* Check to see if the packet contains at least 
     * complete Ethernet (14), IP (20) and TCP/UDP 
     * (8) headers.
     */
    if (n < 42) {
      perror("recvfrom():");
      printf("Incomplete packet (errno is %d)\n",
             errno);
      close(sock);
      exit(0);
    }

    ethhead = buffer;
    printf("Source MAC address: "
           "%02x:%02x:%02x:%02x:%02x:%02x\n",
           ethhead[0],ethhead[1],ethhead[2],
           ethhead[3],ethhead[4],ethhead[5]);
    printf("Destination MAC address: "
           "%02x:%02x:%02x:%02x:%02x:%02x\n",
           ethhead[6],ethhead[7],ethhead[8],
           ethhead[9],ethhead[10],ethhead[11]);

    iphead = buffer + 14; /* Skip Ethernet  header */
    if (*iphead == 0x45) { /* Double check for IPv4 
                            * and no options present */
      printf("Source host %d.%d.%d.%d\n",
             iphead[12],iphead[13],
             iphead[14],iphead[15]);
      printf("Dest host %d.%d.%d.%d\n",
             iphead[16],iphead[17],
             iphead[18],iphead[19]);
      printf("Source,Dest ports %d,%d\n",
             (iphead[20]<<8)+iphead[21],
             (iphead[22]<<8)+iphead[23]);
      printf("Layer-4 protocol %d\n",iphead[9]);

      if (iphead[9] == IPPROTO_UDP) {
          udphead = iphead + 20;
          udp_plen = n - 14 - 20 - 8;

          printf("UDP payload %d bytes:\n", udp_plen);
          dump_raw(udphead+8, udp_plen);
      }
      
    }
  }
  
}

