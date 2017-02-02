// Change the EFFECTIVE_UID value, compile and run the following as root.
// This test tries to set the capability cap_net_raw, then switch from root
// to an effective user, and open a raw socket being not a root.

// gcc -g -o cap_test cap_test.c -lcap
//

#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/prctl.h>

#define EFFECTIVE_UID 501

int main()
{
    int s1, s2, s3, s4, rc;
        cap_t   caps;
        char    cmd [1024];

        if  (prctl (PR_SET_KEEPCAPS, 1, 0, 0, 0) < 0)
            printf ("\nCould not SET_KEEPCAPS\n");
        printf ("Keeping the Capabilities: %d\n",
                prctl (PR_GET_KEEPCAPS, 0,0,0,0));

        printf ("\nReal UID=%d, Real GID=%d, Eff UID=%d, Eff GID=%d\n",
                getuid(), getgid(), geteuid(), getegid());

        caps =  cap_get_proc ();
        printf ("\nInitial Capabilities: %s\n",
                cap_to_text (caps, NULL));

        //sprintf (cmd, "cap_setuid,cap_setgid,cap_net_raw=eip");
        sprintf (cmd, "cap_setuid,cap_setgid,cap_net_raw+ep");
        printf  ("\nSetting the Capabilities %s\n",  cmd);
        rc   =  cap_set_proc (cap_from_text     (cmd));
        if (rc != 0)
           printf ("Failed to set the Capabilities: %s\n",
                   strerror(errno));

        caps =  cap_get_proc ();
        printf ("\nPrivileged Capabilities: %s\n",
                cap_to_text(caps, NULL));


        s1 = socket (PF_INET, SOCK_RAW, IPPROTO_UDP);
        if (s1 < 0)
            printf ("PF_INET error: %s\n", strerror (errno));

        s2 = socket (PF_PACKET, SOCK_RAW,  htonl(ETH_P_IP));
        if (s2 < 0)
            printf ("PF_INET error: %s\n", strerror (errno));


        setegid (EFFECTIVE_UID);
        seteuid (EFFECTIVE_UID);

        printf ("\nReal UID=%d, Real GID=%d, Eff UID=%d, Eff GID=%d\n",
                getuid(), getgid(), geteuid(), getegid());

        caps =  cap_get_proc ();
        printf ("\nUnPrivileged Capabilities: %s\n",
                cap_to_text(caps, NULL));

        s1 = socket (PF_INET, SOCK_RAW, IPPROTO_UDP);
        if (s1 < 0)
            printf ("PF_INET error: %s\n", strerror (errno));

        s2 = socket (PF_PACKET, SOCK_RAW,  htonl(ETH_P_IP));
        if (s2 < 0)
            printf ("PF_INET error: %s\n", strerror (errno));

        return 0;
}
//----------------------------[ End test ]---------------------------- 

