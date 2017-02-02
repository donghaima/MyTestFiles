#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>

static 
char *inet_ntoa_r(struct in_addr saddr, char *buf, int buf_len)
{
    return ((char *)inet_ntop(AF_INET, &saddr, buf, buf_len));
}

static inline
char *uint32_ntoa_r (uint32_t addr, char *buf, int buf_len)
{
    struct in_addr saddr;

    saddr.s_addr = addr;
    return ((char *)inet_ntop(AF_INET, &saddr, buf, buf_len));
}


int main(void)
{
    char str[INET_ADDRSTRLEN];
    char foo_p[] = "228.1.1.1";
    struct in_addr foo_in_addr;

    printf("Use inet_addr() and inet_ntoa():\n");
    foo_in_addr.s_addr = inet_addr(foo_p);
    printf("  addr_n=0x%x, addr_p=%s\n", foo_in_addr.s_addr,
           inet_ntoa(foo_in_addr));
    
    foo_in_addr.s_addr = htonl(ntohl(inet_addr(foo_p)) + 1);
    //foo_in_addr.s_addr = 1;
    printf("  addr_n+1=0x%x, addr_p=%s\n", foo_in_addr.s_addr,
           inet_ntoa(foo_in_addr));
    
    printf("\nChange to use inet_pton() and inet_ntop(): \n");
    inet_pton(AF_INET, foo_p, &foo_in_addr);
    printf("  addr_n=0x%x, addr_p=%s\n", 
           foo_in_addr.s_addr,
           inet_ntop(AF_INET, &foo_in_addr, str, sizeof(str)));
    
    printf("\nTest inet_ntoa_r(): addr_p=%s\n",
           inet_ntoa_r(foo_in_addr, str, sizeof(str)));

    inet_ntoa_r(foo_in_addr, str, sizeof(str));
    printf("\nTest inet_ntoa_r(): -2- addr_p=%s\n", str);
    
    printf("\nTest uint32_ntoa_r(): addr_p=%s\n",
           uint32_ntoa_r(foo_in_addr.s_addr, str, sizeof(str)));
 
    return 0;
}
