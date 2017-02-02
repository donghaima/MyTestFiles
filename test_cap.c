/* Need to link to libcap library: -lcap */

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


/* Use the default values when we are not able to retrieve them 
 * programatically
 */
#define DEFAULT_VQES_GID 499
#define DEFAULT_VQES_UID 499


/* Returns 0 on success: uid and gid are returned;
 * Returns -1 on error.
 */
static int get_ugid (char *user, char *group, int *uid, int *gid)
{
    int sw_uid;
    int sw_gid;
    char *endp;
    struct group *gr;
    struct passwd *pw;
    
    if (!user || !group) {
        return -1;
    }

    if (isdigit((unsigned char)*user)) {
        sw_uid = (uid_t)strtoul(user, &endp, 0);
        if (*endp != '\0') 
            goto getuser;
            
        if ((pw = getpwuid(sw_uid)) != NULL) {
            user = strdup(pw->pw_name);
        } else {
            user = (char *)-1;
        }
            
    } else {
    getuser:	
        if ((pw = getpwnam(user)) != NULL) {
            sw_uid = pw->pw_uid;
            sw_gid = pw->pw_gid;
        } else {
            errno = 0;
            printf("Cannot find user `%s'\n", user);
            return (-1);
        }
    }

    if (isdigit((unsigned char)*group)) {
        sw_gid = (gid_t)strtoul(group, &endp, 0);
        if (*endp != '\0') 
            goto getgroup;
    } else {
    getgroup:	
        if ((gr = getgrnam(group)) != NULL) {
            sw_gid = gr->gr_gid;
        } else {
            errno = 0;
            printf("Cannot find group `%s'\n", group);
            return (-1);
        }
    }

    if (user && (user != (char *)-1) && initgroups(user, sw_gid)) {
        printf("Cannot initgroups() to user `%s': %m\n", user);
        return (-1);
    }

    *uid = sw_uid;
    *gid = sw_gid;
    return 0;
}


/* Change to non-root user uid/gid and drop all capabilities except for 
 * CAP_NET_RAW. 
 */
void change_ugid (char *user, char *group)
{
    /* Only need to change if we are root/root */
    if ((getuid() == 0) && (getgid()==0)) {
        int sw_uid, sw_gid;
        cap_t caps;
        
        /* Set flag and keep privileges accross setuid() call (we only really 
         * need cap_net_raw): */
        if (prctl(PR_SET_KEEPCAPS, 1L, 0L, 0L, 0L) < 0)
            printf ("\nCould not SET_KEEPCAPS\n");

        printf("Keeping the Capabilities: %d\n",
               prctl(PR_GET_KEEPCAPS, 0,0,0,0));

        if (get_ugid(user, group, &sw_uid, &sw_gid)) {
            /* Log an information log: use default VQES uid and gid */
            printf("To use the default uid=%d, gid=%d\n", 
                   DEFAULT_VQES_UID, DEFAULT_VQES_GID);

            sw_uid = DEFAULT_VQES_UID;
            sw_gid = DEFAULT_VQES_UID;
        }

        printf ("Original real UID=%d,GID=%d, effective UID=%d,GID=%d\n",
                getuid(), getgid(), geteuid(), getegid());
        
        if (setgid(sw_gid) || setegid(sw_gid) || 
            setuid(sw_uid) || seteuid(sw_uid)) {
            printf("Cannot setgid() to group `%s': %m", group);
            goto not_changed;
        }
        
        printf ("Set to real UID=%d,GID=%d, effective UID=%d,GID=%d\n",
                getuid(), getgid(), geteuid(), getegid());

        /* Drop the full root privileges, except for the cap_net_raw */
        caps = cap_get_proc();
        printf("Initial Capabilities: %s\n", cap_to_text (caps, NULL));

        if( !(caps = cap_from_text("cap_net_raw=ipe"))) {
            printf("cap_from_text() failed: %m\n");
            goto not_changed;
        }
        if( cap_set_proc(caps) == -1 ) {
            printf("cap_set_proc() failed to drop root privileges: %m\n" );
            goto not_changed;
        }

        caps =  cap_get_proc();
        printf ("Capabilities set to: %s\n", cap_to_text(caps, NULL));

        cap_free(caps);
        
        return;
    }

not_changed:
    /* Log a notice */

    return;
}






int main(void)
{
    int s1, s2;

    char *user = "vqes";		/* User to switch to */
    char *group = "vqes";		/* group to switch to */


    change_ugid(user, group);


    /* Try to open some raw sockets: test the CAP_NET_RAW */
    s1 = socket (PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (s1 < 0) {
        printf ("PF_INET error: %s\n", strerror (errno));
        
    }

    s2 = socket (PF_PACKET, SOCK_RAW,  htonl(ETH_P_IP));
    if (s2 < 0) {
        printf ("PF_PACKET error: %s\n", strerror (errno));
    
    }

    printf("Created a RAW and a PACKET sockets\n");


    printf("\nNow try to open a system file\n");
    system("cat /etc/shadow");
    
    return 0;
}



