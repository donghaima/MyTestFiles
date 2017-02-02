#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

/*
 * Given an IP address in in_addr_t format, return a dot separated 
 * IP address string.
 */
char *in_addr_t_ntoa(in_addr_t addr)
{
    static char pbuf[16+1];
    char *s = pbuf;
    int i, len = 4;

    if (addr == 0) {
        strcpy (s, "<null address>");
    }
    for (i = 0; i < len; i++) {
        sprintf (s, "%s%d", i ? "." : "", (addr >> (24-i*8)) & 0xFF);
        s += strlen (s);
    }

    return pbuf;
}


/*
 * Given an uint32_t IP address in host format, return a dot separated 
 * IP address string.
 */
char *uint32_htoa(uint32_t addr)
{
    static char pbuf[16+1];
    char *s = pbuf;
    int i, len = 4;

    if (addr == 0) {
        strcpy (s, "<null address>");
    }
    for (i = 0; i < len; i++) {
        sprintf (s, "%s%d", i ? "." : "", (addr >> i*8) & 0xFF);
        s += strlen (s);
    }

    return pbuf;
}


int main(void)
{
    in_addr_t addr1 = 0x0a0a1617;
    in_addr_t addr2 = 0xe0abe301;

    uint32_t h_addr1 = 0x17160a0a;
    uint32_t h_addr2 = 0x01e3abe0;

    printf("addr1=0x%x, str=%s\n", addr1, in_addr_t_ntoa(addr1));
    printf("addr2=0x%x, str=%s\n", addr2, in_addr_t_ntoa(addr2));

    printf("h_addr1=0x%x, str=%s\n", addr1, uint32_htoa(h_addr1));
    printf("h_addr2=0x%x, str=%s\n", addr2, uint32_htoa(h_addr2));


}
