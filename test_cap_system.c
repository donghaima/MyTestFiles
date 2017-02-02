
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>

typedef enum vqe_cap_ret_ {
    VQE_CAP_OK,
    VQE_CAP_PRCTL,
    VQE_CAP_TEXTCAP,
    VQE_CAP_SETCAP,
    VQE_CAP_SETUID,
    VQE_CAP_SETGID,
    VQE_CAP_OTHER
} vqe_cap_ret_t;

/* Grants effective, inherited, and permitted CAP_NET_ADMIN capability */
#define VQE_CAP_CAPNETADMIN_EIP "cap_net_admin=eip"

/* Grants effective, inherited, and permitted CAP_NET_RAW capability */
#define VQE_CAP_CAPNETRAW_EIP "cap_net_raw=eip"


typedef uint8_t boolean;
#define FALSE 0;
#define TRUE (!FALSE);

/*
 * See documentation in vqe_caps.h
 */
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


int main (void)
{
    int sw_uid = 499;
    int sw_gid = 499;
    
    vqe_cap_ret_t ret = vqe_keep_cap(sw_uid, sw_gid, VQE_CAP_CAPNETADMIN_EIP);
    if (ret != VQE_CAP_OK) {
        fprintf(stderr, "Failed to drop capability: %d\n", ret);
        return -1;
    }

    printf("\nNow try to open a system file\n");
    system("cat /etc/shadow");

    printf("\nNow try to show ip addr\n");
    system("/sbin/ip -f inet -o addr show dev lo");

    printf("\nNow try to show ip addr\n");
    system("/opt/vqes/bin/fbt_flush.sh >> /tmp/test_flush_fbt.log");

    return 0;
}

