/* setsndrcv.c:
 *
 * Set SO_SNDBUF & SO_RCVBUF Options:
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

#include <sys/capability.h> /* Must compile w/ -lcap */
#include <sys/prctl.h>
#include <netinet/ip.h>

typedef uint8_t boolean;
#define FALSE 0
#define TRUE  1

typedef enum vqe_cap_ret_ {
    VQE_CAP_OK,
    VQE_CAP_PRCTL,
    VQE_CAP_TEXTCAP,
    VQE_CAP_SETCAP,
    VQE_CAP_SETUID,
    VQE_CAP_SETGID,
    VQE_CAP_OTHER
}vqe_cap_ret_t;

vqe_cap_ret_t vqe_keep_cap (uid_t new_uid, 
                            gid_t new_gid, 
                            const char *cap_string)
{
    boolean retval = FALSE;
    cap_t caps; 

    /* Create cap object from text string, be sure to keep CAP_SETUID */
    caps = cap_from_text(cap_string);
    if (!caps) {
        retval = VQE_CAP_TEXTCAP;
        goto bail;
    }

    /* Set option to keep capabilities through a UID/GID change */
    if (prctl(PR_SET_KEEPCAPS, 1) != 0) {
        retval = VQE_CAP_PRCTL;
        goto bail;
    }

    /* Drop capabilities to a lower user/group */
    if (setgid(new_gid) != 0) {
        retval = VQE_CAP_SETGID;
        goto bail;
    }

    if (setuid(new_uid) != 0) {
        retval = VQE_CAP_SETUID;
        goto bail;
    }

    /* Set capabilities to given set */
    if (cap_set_proc(caps) == -1) {
        retval = VQE_CAP_SETCAP;
        goto bail;
    }
    
bail:
    if (caps)
        cap_free(caps);

    return (retval);
}


/*
 * This function reports the error and
 * exits back to the shell: <$nopage>
 */
static void
bail (const char *on_what) 
{
    if ( errno != 0 ) {
        fputs(strerror(errno),stderr);
        fputs(": ",stderr);
    }
    fputs(on_what,stderr);
    fputc('\n',stderr);
    exit(1);
}


#define VQE_CAP_CAPNETADMIN_CAPNETRAW_EIP "cap_net_raw,cap_net_admin=eip"
#define VQE_CAP_CAPNETADMIN_EIP "cap_net_admin=eip"
#define VQE_CAP_CAPNETRAW_EIP "cap_net_raw=eip"


int
main (int argc,char **argv) 
{
    int z;
    int s = -1;                /* Socket */
    int sndbuf=0;   /* Send buffer size */
    int rcvbuf=0;/* Receive buffer size */
    socklen_t optlen;  /* Option length */
    vqe_cap_ret_t cap_ret;

    cap_ret = vqe_keep_cap(449, 449,
                           VQE_CAP_CAPNETRAW_EIP);
    if (cap_ret != VQE_CAP_OK) {
        fprintf(stderr, "capability drop failure: %d\n", cap_ret);
        return(-1);
    }

    printf("Current uid=%d, gid=%d\n", getuid(), getgid());

    
    /*
     * Create a TCP/IP socket to use:
     */
    //s = socket(PF_INET,SOCK_STREAM,0);
    s = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if ( s == -1 )
        bail("socket(2)");
    
    /*
     * Set the SO_SNDBUF Size:
     */
    sndbuf = 5000;  /* Send buffer size */
    z = setsockopt(s,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof sndbuf);
    if ( z )
        bail("setsockopt(s,SOL_SOCKET,SO_SNDBUF)");
    
    /*
     * Set the SO_RCVBUF Size: <$nopage>
     */
    rcvbuf = 8192;  /* Send buffer size */
    z = setsockopt(s,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof rcvbuf);
    if ( z )
        bail("setsockopt(s,SOL_SOCKET,SO_RCVBUF)");
    
    /*
     * As a check on the above
     * Get socket option SO_SNDBUF:
     */
    optlen = sizeof sndbuf;
    z = getsockopt(s,SOL_SOCKET,SO_SNDBUF,&sndbuf,&optlen);
    if ( z )
        bail("getsockopt(s,SOL_SOCKET,SO_SNDBUF)");
    
    assert(optlen == sizeof sndbuf);
    
    /*
     * Get socket option SO_SNDBUF:
     */
    optlen = sizeof rcvbuf;
    z = getsockopt(s,SOL_SOCKET,SO_RCVBUF, &rcvbuf,&optlen);
    if ( z )
        bail("getsockopt(s,SOL_SOCKET, SO_RCVBUF)");
    
    assert(optlen == sizeof rcvbuf);

    /*
     * Report the buffer sizes: <$nopage>
     */
    printf("Socket s : %d\n",s);
    printf(" Send buf: %d bytes\n", sndbuf);
    printf(" Recv buf: %d bytes\n", rcvbuf);
    
    close(s);
    return 0;
 }
