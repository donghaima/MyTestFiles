#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#define    LOG_EX_SYSLOG (25<<3)    /* Example */
#define    LOG_VQES_CP 	 (24<<3)    /* VQE-S Control Plane */
#define    LOG_VQES_DP 	 (26<<3)    /* VQE-S Data Plane */
#define    LOG_LIBRTP 	 (27<<3)    /* RTP/RTCP library */

int main(int argc, char *argv[])
{
    int priority = LOG_DEBUG;
    if (argc == 2)
	priority = atoi(argv[1]);
    if (priority > LOG_DEBUG)
	priority = LOG_DEBUG;


    openlog ("log_local0", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    //openlog ("log_vqes_cp", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_VQES_CP);
    //openlog ("log_vqes_cp", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_VQES_CP);

    int LogMask = LOG_UPTO (priority);
    setlogmask (LogMask);

    printf("prioity=%d, LOG_UPTO(%d)=%d, LogMask=%d\n", 
           priority, priority, LOG_UPTO(priority), LogMask);

    printf("original check=%d, dc-os check=%d\n",
           (!LOG_MASK(LOG_PRI(priority)) & LogMask),
           ((LOG_MASK(LOG_PRI(priority)) & LogMask) == 0));


    syslog(LOG_DEBUG, "LOG_DEBUG: test test");
    syslog(LOG_ERR, "LOG_ERR: test test");
    
#if 0    
    syslog (LOG_MAKEPRI(LOG_USER, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_USER, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_USER, LOG_INFO),
            "LOG_MAKEPRI: LOG_USER, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR),
            "LOG_MAKEPRI: LOG_USER, LOG_ERR");

    syslog (LOG_MAKEPRI(LOG_DAEMON, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_DAEMON, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_DAEMON, LOG_INFO),
            "LOG_MAKEPRI: LOG_DAEMON, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_DAEMON, LOG_ERR),
            "LOG_MAKEPRI: LOG_DAEMON, LOG_ERR");

    syslog (LOG_MAKEPRI(LOG_LOCAL0, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_LOCAL0, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_LOCAL0, LOG_INFO),
            "LOG_MAKEPRI: LOG_LOCAL0, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_LOCAL0, LOG_ERR),
            "LOG_MAKEPRI: LOG_LOCAL0, LOG_ERR");
#endif


#if 0
    syslog (LOG_MAKEPRI(LOG_LOCAL1, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_LOCAL1, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_LOCAL1, LOG_INFO),
            "LOG_MAKEPRI: LOG_LOCAL1, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR),
            "LOG_MAKEPRI: LOG_LOCAL1, LOG_ERR");

    syslog (LOG_MAKEPRI(LOG_LOCAL4, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_LOCAL4, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_LOCAL4, LOG_INFO),
            "LOG_MAKEPRI: LOG_LOCAL4, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_LOCAL4, LOG_ERR),
            "LOG_MAKEPRI: LOG_LOCAL4, LOG_ERR");

    syslog (LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG),
            "LOG_MAKEPRI: LOG_LOCAL7, LOG_DEBUG");
    syslog (LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO),
            "LOG_MAKEPRI: LOG_LOCAL7, LOG_INFO");
    syslog (LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR),
            "LOG_MAKEPRI: LOG_LOCAL7, LOG_ERR");
#endif

    return 0;
}
