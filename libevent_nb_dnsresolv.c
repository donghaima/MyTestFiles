#include <sys/types.h>
#include <sys/time.h>

/* For inet_ntoa. */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <event.h>
#include <evdns.h>

void
usage(void)
{
        fprintf(stderr, USAGE: evdns-demo <hostname>n);
        exit(1);
}

void
evdns_cb(int result, char type, int count, int ttl, void *addresses, void *arg)
{
        struct in_addr *addrs = addresses;
        int i;

        if (result != 0) {
                printf(Error looking up address.n);
                exit(1);
        }
        else {
                for (i = 0; i < count; i++) {
                        printf(%sn, inet_ntoa(addrs[i]));
                }
                exit(0);
        }
}

int
main(int argc, char **argv)
{
        if (argc != 2)
                usage();

        event_init();
        evdns_init();
        evdns_resolve_ipv4(argv[1], 0, evdns_cb, NULL);
        event_dispatch();

        return (0);
}

