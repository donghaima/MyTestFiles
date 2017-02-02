
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/prctl.h>

int main(void)
{
    int s1=-1, s2=-1;

    int sw_uid = 499;
    int sw_gid = 499;

    /* set flag: keep privileges accross setuid() call (we only really 
     * need cap_net_raw): */
    if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) < 0)
        printf ("\nCould not SET_KEEPCAPS\n");

    printf("Keeping the Capabilities: %d\n",
           prctl(PR_GET_KEEPCAPS, 0,0,0,0));

    printf ("\nOriginal real UID=%d,GID=%d, effective UID=%d,GID=%d\n",
            getuid(), getgid(), geteuid(), getegid());


    s1 = socket (PF_INET, SOCK_RAW, IPPROTO_RAW);
    if (s1 < 0) {
        printf ("PF_INET error: %s\n", strerror(errno));
    } else 
        printf("Created a RAW sockets\n");

    
    /* */
    if (setgid(sw_gid)) {
        printf("Cannot setgid() to group %d: %m", sw_gid);
        return (-1);
    }
    if (setegid(sw_gid)) {
        printf("Cannot setegid() to %d: %m", sw_gid);
        return (-1);
    }
    if (setuid(sw_uid)) {
        printf("Cannot setuid() to %d: %m", sw_uid);
        return (-1);
    }
    if (seteuid(sw_uid)) {
        printf("Cannot seteuid() to %d: %m", sw_uid);
        return (-1);
    }

    printf ("\nSet to real UID=%d,GID=%d, effective UID=%d,GID=%d\n",
            getuid(), getgid(), geteuid(), getegid());


#define CHANGE_CAP
#ifdef CHANGE_CAP
    /* We may be running under non-root uid now, but we still hold 
     * full root privileges!
     * We drop all of them, except for the cap_net_raw.
     */
    cap_t caps;

    caps = cap_get_proc();
    printf("\nInitial Capabilities: %s\n",
           cap_to_text (caps, NULL));

    if( ! ( caps = cap_from_text( "cap_net_raw=ipe" ) ) ) {
        //if( ! ( caps = cap_from_text( "cap_net_raw+pe" ) ) ) {
        printf("cap_from_text() failed: %m\n");
        return -1;
    }
    if( cap_set_proc( caps ) == -1 ) {
        printf("cap_set_proc() failed to drop root privileges: %m\n" );
        return -1;
    }

    caps =  cap_get_proc();
    printf ("\nCapabilities set to: %s\n",
            cap_to_text(caps, NULL));

    cap_free( caps );
#endif /* #ifdef CHANGE_CAP */



    s2 = socket (PF_PACKET, SOCK_RAW,  htonl(ETH_P_IP));
    if (s2 < 0) {
        printf ("PF_PACKET error: %s\n", strerror (errno));
    
    } else 
        printf("Created a PACKET sockets\n");

    printf("\nNow try to open a system file\n");
    system("cat /etc/shadow");
    
    return 0;
}

