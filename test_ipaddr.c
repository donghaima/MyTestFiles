#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

#define IPADDR(b0,b1,b2,b3) (htonl((b0 << 24) | (b1 << 16) | (b2 << 8) | b3))

int main(int argc, const char * argv[])
{
    int i;
    uint8_t iparg[4];
    uint32_t s_addr;
   
    if (argc < 5) {
        printf ("Usage: %s addr(e.g. 224 1 1 1)\n", argv[0]); 
        return -1;
    }

    for (i = 0; i < 4; i ++) {
        iparg[i] = atoi(argv[i+1]);
    }

    s_addr = IPADDR(iparg[0],iparg[1],iparg[2],iparg[3]);

    printf("iparg[]=%d, %d, %d, %d; s_addr=0x%x\n",
           iparg[0], iparg[1], iparg[2], iparg[3], s_addr);
    
    return 0;
}
