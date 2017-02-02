#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>


int main(void)
{
    int sock;
    const char *addr_str = "10.86.21.28";
    int port_num = 0;
    in_port_t port = htons(port_num);
    in_addr_t addr = inet_addr(addr_str);

    int on = 1, ret;
    struct sockaddr_in	servaddr, sin;
    socklen_t socklen = sizeof(sin);

    printf("To use addr: %s:%d\n", addr_str, port_num);

    /* Create a rudimentary socket */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("open");
        return(-1);
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        perror("setsockopt");
        goto bail;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = 0;   /* ask Linux to pick a port */

    ret = bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind");
        goto bail;
    }

    memset(&sin, 0, sizeof(sin));
    ret = getsockname(fd, (struct sockaddr *)&sin, &socklen);
    if (ret == -1) {
        perror("getsockname");
        goto bail;        
    }

    printf("Created a listening socket on %s:%d\n", 
           inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    //sleep(10);
 bail:
    close(fd);
    return ret;
}

