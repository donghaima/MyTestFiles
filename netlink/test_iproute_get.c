/* Do ip route get */

#include <stdio.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/in_route.h>
#include <linux/ip_mp_alg.h>

#include "utils.h"
#include "libnetlink.h"

#define NEXT_ARG() \
    do { argv++; if (--argc <= 0) incomplete_command(); } while(0)
#define NEXT_ARG_OK() (argc - 1 > 0)
#define PREV_ARG() do { argv--; argc++; } while(0)


static struct rtnl_handle rth = { .fd = -1 };


static void usage (void)
{
    fprintf(stderr, "Usage: test_iproute_get \n");
    exit(-1);
}



static int print_route (const struct sockaddr_nl *who, 
                        struct nlmsghdr *n, void *arg)
{
    FILE *fp = (FILE*)arg;
    struct rtmsg *r = NLMSG_DATA(n);
    int len = n->nlmsg_len;
    struct rtattr *tb[RTA_MAX+1];
    int host_len = -1;
    __u32 table;

    /* Parse the returned netlink msg */
    if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
        fprintf(stderr, "Not a route: %08x %08x %08x\n",
                n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
        return 0;
    }

    len -= NLMSG_LENGTH(sizeof(*r));
    if (len < 0) {
        fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
        return -1;
    }

    if (r->rtm_family == AF_INET6)
        host_len = 128;
    else if (r->rtm_family == AF_INET)
        host_len = 32;
    else if (r->rtm_family == AF_DECnet)
        host_len = 16;
    else if (r->rtm_family == AF_IPX)
        host_len = 80;

    parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
    table = rtm_get_table(r, tb);


    fprintf(fp, "Done\n");
    fflush(fp);

    return 0;
}




int main (int argc, char **argv)
{
    struct {
        struct nlmsghdr 	n;
        struct rtmsg 		r;
        char   			buf[1024];
    } req;
    char  *idev = NULL;
    char  *odev = NULL;
    int connected = 0;
    int from_ok = 0;

    memset(&req, 0, sizeof(req));
    
    //iproute_reset_filter();

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_GETROUTE;
    req.r.rtm_family = AF_INET;
    req.r.rtm_table = 0;
    req.r.rtm_protocol = 0;
    req.r.rtm_scope = 0;
    req.r.rtm_type = 0;
    req.r.rtm_src_len = 0;
    req.r.rtm_dst_len = 0;
    req.r.rtm_tos = 0;

    /* skip argv[0] */
    NEXT_ARG();

    while (argc > 0) {
        if (strcmp(*argv, "tos") == 0 ||
            matches(*argv, "dsfield") == 0) {
            __u32 tos;
            NEXT_ARG();
            /* No validation for the TOS value */
            //if (rtnl_dsfield_a2n(&tos, *argv))
            //   invarg("TOS value is invalid\n", *argv);
            req.r.rtm_tos = tos;
        } else if (matches(*argv, "from") == 0) {
            inet_prefix addr;
            NEXT_ARG();
            if (matches(*argv, "help") == 0)
                usage();
            from_ok = 1;
            get_prefix(&addr, *argv, req.r.rtm_family);
            if (req.r.rtm_family == AF_UNSPEC)
                req.r.rtm_family = addr.family;
            if (addr.bytelen)
                addattr_l(&req.n, sizeof(req), RTA_SRC, &addr.data, addr.bytelen);
            req.r.rtm_src_len = addr.bitlen;
        } else if (matches(*argv, "iif") == 0) {
            NEXT_ARG();
            idev = *argv;
        } else if (matches(*argv, "oif") == 0 ||
                   strcmp(*argv, "dev") == 0) {
            NEXT_ARG();
            odev = *argv;
        } else if (matches(*argv, "notify") == 0) {
            req.r.rtm_flags |= RTM_F_NOTIFY;
        } else if (matches(*argv, "connected") == 0) {
            connected = 1;
        } else {
            inet_prefix addr;
            if (strcmp(*argv, "to") == 0) {
                NEXT_ARG();
            }
            if (matches(*argv, "help") == 0)
                usage();
            get_prefix(&addr, *argv, req.r.rtm_family);
            if (req.r.rtm_family == AF_UNSPEC)
                req.r.rtm_family = addr.family;
            if (addr.bytelen)
                addattr_l(&req.n, sizeof(req), RTA_DST, &addr.data, addr.bytelen);
            req.r.rtm_dst_len = addr.bitlen;
        }
        argc--; argv++;
    }
    
    if (req.r.rtm_dst_len == 0) {
        fprintf(stderr, "need at least destination address\n");
        exit(1);
    }


    /* Open the netlink socket */
    if (rtnl_open(&rth, 0) < 0)
        exit(1);


#define SUPPORT_DEV 1

#if SUPPORT_DEV
    
    if (idev || odev)  {
        int idx;

        ll_init_map(&rth);
        
        if (idev) {
            if ((idx = ll_name_to_index(idev)) == 0) {
                fprintf(stderr, "Cannot find device \"%s\"\n", idev);
                return -1;
            }
            addattr32(&req.n, sizeof(req), RTA_IIF, idx);
        }
        if (odev) {
            if ((idx = ll_name_to_index(odev)) == 0) {
                fprintf(stderr, "Cannot find device \"%s\"\n", odev);
                return -1;
            }
            addattr32(&req.n, sizeof(req), RTA_OIF, idx);
        }
    }
#endif
    
    if (req.r.rtm_family == AF_UNSPEC)
        req.r.rtm_family = AF_INET;
    

    /* Measure the time rtnl_talk() takes */
    {
        int usec;
        struct timeval start, finish;
        
        gettimeofday (&start, NULL);
        if (rtnl_talk(&rth, &req.n, 0, 0, &req.n, NULL, NULL) < 0) {
            exit(2);
        }
        gettimeofday (&finish, NULL);
        usec = finish.tv_sec * 1000000 + finish.tv_usec;
        usec -= start.tv_sec * 1000000 + start.tv_usec;
        printf("rtnl_talk() took %d usecs\n", usec);
    }


    if (print_route(NULL, &req.n, (void*)stdout) < 0) {
        fprintf(stderr, "An error :-)\n");
        exit(1);
    }

    rtnl_close(&rth);
    exit(0);
}
