/* 8-04-2005, jharan,

   stolen from:
   http://mail.nl.linux.org/kernelnewbies/2003-12/msg00138.html

   then modified

   Hi all,
   i am including a piece of code i have written to get the network
   interfaces
   and their IP addresses using rtnetlink. Please do let me know if there
   is a better way of doing this.

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <unistd.h>

#define NL_BUFSIZE 8192

typedef struct {
    __u16 nlmsg_type;
    char *name;
} nlmsg_type_name;

#define NLMSG_NAME_INIT(name) { name, #name }

nlmsg_type_name nlmsg_type_table[] = {
    NLMSG_NAME_INIT(NLMSG_NOOP),
    NLMSG_NAME_INIT(NLMSG_ERROR),
    NLMSG_NAME_INIT(NLMSG_DONE),
    NLMSG_NAME_INIT(NLMSG_OVERRUN),
    NLMSG_NAME_INIT(RTM_NEWLINK),
    NLMSG_NAME_INIT(RTM_DELLINK),
    NLMSG_NAME_INIT(RTM_GETLINK),
    NLMSG_NAME_INIT(RTM_NEWADDR),
    NLMSG_NAME_INIT(RTM_DELADDR),
    NLMSG_NAME_INIT(RTM_GETADDR),
    NLMSG_NAME_INIT(RTM_NEWROUTE),
    NLMSG_NAME_INIT(RTM_DELROUTE),
    NLMSG_NAME_INIT(RTM_GETROUTE),
    NLMSG_NAME_INIT(RTM_NEWNEIGH),
    NLMSG_NAME_INIT(RTM_DELNEIGH),
    NLMSG_NAME_INIT(RTM_GETNEIGH),
    NLMSG_NAME_INIT(RTM_NEWRULE),
    NLMSG_NAME_INIT(RTM_DELRULE),
    NLMSG_NAME_INIT(RTM_GETRULE),
    NLMSG_NAME_INIT(RTM_NEWQDISC),
    NLMSG_NAME_INIT(RTM_DELQDISC),
    NLMSG_NAME_INIT(RTM_GETQDISC),
    NLMSG_NAME_INIT(RTM_NEWTCLASS),
    NLMSG_NAME_INIT(RTM_DELTCLASS),
    NLMSG_NAME_INIT(RTM_GETTCLASS),
    NLMSG_NAME_INIT(RTM_NEWTFILTER),
    NLMSG_NAME_INIT(RTM_DELTFILTER),
    NLMSG_NAME_INIT(RTM_GETTFILTER),
    NLMSG_NAME_INIT(RTM_MAX),
    { 0, 0 } /* end of table marker */
};

char *lookup_nlmsg_type(__u16 nlmsg_type)
{
    int i;

    for (i = 0; nlmsg_type_table[i].name; ++i) {
        if (nlmsg_type_table[i].nlmsg_type == nlmsg_type)
            return nlmsg_type_table[i].name;
    }
    return ("unknown nlmsg_type");
}

typedef struct {
    int nlmsg_flag;
    char *name;
} nlmsg_flags_name;

#define NLMSG_FLAGS_INIT(name) { name, #name }

nlmsg_flags_name nlmsg_flags_table[] = {
    NLMSG_FLAGS_INIT(NLM_F_REQUEST),
    NLMSG_FLAGS_INIT(NLM_F_MULTI),
    NLMSG_FLAGS_INIT(NLM_F_ACK),
    NLMSG_FLAGS_INIT(NLM_F_ECHO),
    NLMSG_FLAGS_INIT(NLM_F_ROOT),
    NLMSG_FLAGS_INIT(NLM_F_MATCH),
    NLMSG_FLAGS_INIT(NLM_F_ATOMIC),
    NLMSG_FLAGS_INIT(NLM_F_REPLACE),
    NLMSG_FLAGS_INIT(NLM_F_EXCL),
    NLMSG_FLAGS_INIT(NLM_F_CREATE),
    NLMSG_FLAGS_INIT(NLM_F_APPEND),
    { 0, 0 } /* end of table marker */
};

__u16 print_nlmsg_flags(__u16 nlmsg_flags)
{
    int i;

    /* while some flag bits are still set */
    while (nlmsg_flags) {
        /* search table for a flag where we know what it is */
        for (i = 0; nlmsg_flags_table[i].name; ++i) {
            /* known flag? */
            if (nlmsg_flags_table[i].nlmsg_flag &
                nlmsg_flags) {
                /* print its name */
                fprintf(stdout, "%s ",
                        nlmsg_flags_table[i].name); 
                /* reset the flag */
                nlmsg_flags &=
                    ~nlmsg_flags_table[i].nlmsg_flag;
                break;
            }
        }
        /* if we couldn't find any matching flags, bail */
        if (!nlmsg_flags_table[i].name)
            break;
    }
    /* if any unknown flags left, display them numerically */
    if (nlmsg_flags)
        fprintf(stdout, "mystery flags 0x%x", nlmsg_flags);
    return nlmsg_flags;
}


int readSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
    struct nlmsghdr *nlHdr;
    int readLen = 0, flag = 0, msgLen = 0;

    do{
        /* Recieve response from kernel */
        if ((readLen = recv(sockFd, bufPtr, NL_BUFSIZE - msgLen,
                            0)) < 0){
            perror("SOCK READ: ");
            return -1;
        }
        nlHdr = (struct nlmsghdr *)bufPtr;

        /* Check if header is valid */
        if ((NLMSG_OK(nlHdr, readLen) == 0) ||
            (nlHdr->nlmsg_type == NLMSG_ERROR)){
            perror("Error in recieved packet");
            return -1;
        }

        fprintf(stdout, "nlmsg_type = %d %s\n",
                nlHdr->nlmsg_type,
                lookup_nlmsg_type(nlHdr->nlmsg_type));
        fprintf(stdout, "nlmsg_flags = 0x%x ",
                nlHdr->nlmsg_flags);
        print_nlmsg_flags(nlHdr->nlmsg_flags);
        fprintf(stdout, "\n");

        /* Check if its the last message */
        if (nlHdr->nlmsg_type == NLMSG_DONE) {
            flag = 1;
            break;
        } else {
            /* Move the buffer pointer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }
#if 0
        /* jharan, though it seems to work, not clear to me that the
           following
           is "correct". I think we should only be looking for the
           NLMSG_DONE
           to terminate, like is done above */
        if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0){
            flag = 1;
            break;
        }
#endif
    } while ((nlHdr->nlmsg_seq != seqNum) ||
             (nlHdr->nlmsg_pid != pId) || (flag == 0));
    return msgLen;
}

void parseIfAddr(struct nlmsghdr *nlHdr)
{
    struct ifaddrmsg *ifAddrs;
    struct rtattr *rtAttr;
    int rtLen;
    unsigned char ifa_flags;
    char tempBuf[100];

    ifAddrs = (struct ifaddrmsg *)NLMSG_DATA(nlHdr);

    fprintf(stdout,
            "=============================================\n");
    fprintf(stdout, "ifaddrmsg:ifa_family %d\n",
            ifAddrs->ifa_family);
    if (ifAddrs->ifa_family == AF_INET)
        fprintf(stdout, "\tAF_INET\n");
    else
        fprintf(stdout, "\tmystery family\n");
    fprintf(stdout, "ifaddrmsg:ifa_prefixlen %d\n",
            ifAddrs->ifa_prefixlen);
    fprintf(stdout, "ifaddrmsg:ifa_flags 0x%x\n\t",
            ifAddrs->ifa_flags);
    /* decode and reset flags */
    ifa_flags = ifAddrs->ifa_flags;
    if (ifa_flags & IFA_F_SECONDARY) {
        fprintf(stdout, "IFA_F_SECONDARY, ");
        ifa_flags &= ~IFA_F_SECONDARY;
    }
    if (ifa_flags & IFA_F_DEPRECATED) {
        fprintf(stdout, "IFA_F_DEPRECATED, ");
        ifa_flags &= ~IFA_F_DEPRECATED;
    }
    if (ifa_flags & IFA_F_TENTATIVE) {
        fprintf(stdout, "IFA_F_TENTATIVE, ");
        ifa_flags &= ~IFA_F_TENTATIVE;
    }
    if (ifa_flags & IFA_F_PERMANENT) {
        fprintf(stdout, "IFA_F_PERMANENT, ");
        ifa_flags &= ~IFA_F_PERMANENT;
    }
    /* display remaining flags we couldn't decode */
    fprintf(stdout, "mystery flags 0x%x\n", ifa_flags);

    fprintf(stdout, "ifaddrmsg:ifa_scope %d\n\t",
            ifAddrs->ifa_scope);
    if (ifAddrs->ifa_scope == RT_SCOPE_UNIVERSE)
        fprintf(stdout,"RT_SCOPE_UNIVERSE\n");
    else if (ifAddrs->ifa_scope == RT_SCOPE_SITE)
        fprintf(stdout,"RT_SCOPE_SITE\n");
    else if (ifAddrs->ifa_scope == RT_SCOPE_LINK)
        fprintf(stdout,"RT_SCOPE_LINK\n");
    else if (ifAddrs->ifa_scope == RT_SCOPE_HOST)
        fprintf(stdout,"RT_SCOPE_HOST\n");
    else if (ifAddrs->ifa_scope == RT_SCOPE_NOWHERE)
        fprintf(stdout,"RT_SCOPE_NOWHERE\n");
    else
        fprintf(stdout,"mystery scope\n");

    fprintf(stdout, "ifaddrmsg:ifa_index %d\n", ifAddrs->ifa_index);

    rtAttr = (struct rtattr *)IFA_RTA(ifAddrs);
    rtLen = IFA_PAYLOAD(nlHdr);
    for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen)){
        fprintf(stdout, "rta_type %d:", rtAttr->rta_type);
        switch(rtAttr->rta_type) {
        case IFA_ADDRESS:
            inet_ntop(AF_INET, RTA_DATA(rtAttr), tempBuf,
                      sizeof(tempBuf));
            fprintf(stdout, "IFA_ADDRESS: %s\n",tempBuf);
            break;
        case IFA_LABEL:
            fprintf(stdout, "IFA_LABEL: %s\n",(char *)
                    RTA_DATA(rtAttr));
            break;
        case IFA_BROADCAST:
            inet_ntop(AF_INET, RTA_DATA(rtAttr), tempBuf,
                      sizeof(tempBuf));
            fprintf(stdout, "IFA_BROADCAST: %s\n",tempBuf);
            break;
        case IFA_LOCAL:
            inet_ntop(AF_INET, RTA_DATA(rtAttr), tempBuf,
                      sizeof(tempBuf));
            fprintf(stdout, "IFA_LOCAL: %s\n",tempBuf);
            break;
        default:
            fprintf(stdout, "\n");
            break;
        }
    }
    return;
}

int main()
{
    struct nlmsghdr *nlMsg;
    struct ifaddrmsg *addrMsg;
    char msgBuf[NL_BUFSIZE];
    int sock, len, msgSeq = 0;

    /* Create Socket */
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
        perror("Socket Creation: ");

    memset(msgBuf, 0, NL_BUFSIZE);

    nlMsg = (struct nlmsghdr *)msgBuf;
    addrMsg = (struct ifaddrmsg *)NLMSG_DATA(nlMsg);

    /* For getting interface addresses */
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    nlMsg->nlmsg_type = RTM_GETADDR;
    nlMsg->nlmsg_flags = NLM_F_ROOT | NLM_F_REQUEST;
    nlMsg->nlmsg_seq = msgSeq++;
    nlMsg->nlmsg_pid = getpid();

    if(write(sock, nlMsg, nlMsg->nlmsg_len) < 0){
        printf("Write To Socket Failed...\n");
        return -1;
    }

    if((len = readSock(sock, msgBuf, msgSeq, getpid())) < 0){
        printf("Read From Socket Failed...\n");
        return -1;
    }

    for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len)){
        /* For getting interface addresses */
        parseIfAddr(nlMsg);

    }
    close(sock);
    return 0;
}
