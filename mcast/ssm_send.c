#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Not everyone has the headers for this so improvise */
#ifndef MCAST_JOIN_SOURCE_GROUP
#define MCAST_JOIN_SOURCE_GROUP 46

struct group_source_req
{
    /* Interface index.  */
    uint32_t gsr_interface;

    /* Group address.  */
    struct sockaddr_storage gsr_group;

    /* Source address.  */
    struct sockaddr_storage gsr_source;
};
#endif

int main (int argc, char *argv[])
{
    struct group_source_req group_source_req;
    struct sockaddr_in *group;
    struct sockaddr_in *source;
    int fd = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
    socklen_t socklen = sizeof(struct sockaddr_storage);
    struct sockaddr_in bindaddr;
    u_char loop = 1;

    if (argc != 3) {
        printf("%s <mcast ip addr> <mcast port>\n", argv[0]);
        return (-1);
    }

    /* First bind to the port */
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(9990);
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(fd,(struct sockaddr*)&bindaddr,sizeof(bindaddr));
    /* Now set up the SSM request */
    group_source_req.gsr_interface = 0; /* "any" interface */
    group=(struct sockaddr_in*)&group_source_req.gsr_group;
    source=(struct sockaddr_in*)&group_source_req.gsr_source;
    
    group->sin_family = AF_INET;
    inet_aton(argv[1], &group->sin_addr);
    group->sin_port = 0;    /* Ignored */

    /* Set the source to the name of the socket we created above */
    getsockname(fd,(struct sockaddr *)source, &socklen);

    setsockopt(fd, SOL_IP, MCAST_JOIN_SOURCE_GROUP, &group_source_req,
               sizeof(group_source_req));

    /* Enable reception of our own multicast */
    loop = 1;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    /* Set the TTL on packets to 250 */
    loop=250;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &loop, sizeof(loop));
    /* Now we care about the port we send to */
    group->sin_port = htons(atoi(argv[2]));

    /* Now send packets */
    while(1) {
        sendto(fd, "Hello World", strlen("Hello World"), 0,
               (struct sockaddr*)group, sizeof(struct sockaddr_in));
        sleep(1);
    }
    return 0;
}

