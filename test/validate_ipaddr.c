#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

int main(int argc, const char * argv[])
{
    in_addr_t ia;
   
    if (argc < 2) {
        printf ("Usage: %s addr(e.g. 224.1.1.1)\n", argv[0]); 
        return -1;
    }

    ia = inet_addr(argv[1]);
    if (ia == -1)
        printf("Invalid input ipaddr string=%s\n", argv[1]);
    else
        printf("ipaddr string=%s ia=0x%x\n", argv[1], ia);
    
    return 0;
}
