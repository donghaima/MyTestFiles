/**-----------------------------------------------------------------
 * @brief
 * VQE-S STUN Server.  Main program
 *
 * @file
 * ss_main.c
 *
 * October 2007, Donghai Ma
 *
 * Copyright (c) 2007-2008 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "sys/event.h"
#include "stun.h"
#include "vqe_caps.h"

#include "ss_define.h"
#include "ss_util.h"
#include "ss_cmdopt.h"
#include "socket_filter.h"
#include "raw_socket.h"
#include "raw_packet.h"
#include "ss_stat.h"
#include "ss_lock.h"
#include "ss_xmlrpc.h"
#include "ss_server.h"

#include <asm/types.h>
#include <linux/if_packet.h>

/*------------------------------------------------------------------------*/
/* Among all the C files including it, only ONE C file (say the one where 
 * main() is defined) should instantiate the message definitions described in 
 * XXX_syslog_def.h.   To do so, the designated C file should define the symbol
 * SYSLOG_DEFINITION before including the file.  
 */
#define SYSLOG_DEFINITION
#include <log/stunsvr_syslog_def.h>
#undef SYSLOG_DEFINITION

/* Include base definitions: boolean flags, enum codes */
#include <log/stunsvr_debug.h>
/* Declare the debug array */
#define __DECLARE_DEBUG_ARR__
#include <log/stunsvr_debug_flags.h>
/*------------------------------------------------------------------------*/


/* Function:    stun_socket_read_callback
 * Description: This is the callback routine for the STUN socket event. It is
 *              called from the libevent framework when a message is received
 *              by the STUN request socket. After the request packet passes the
 *              message checking, will it be processed by the STUN request 
 *              handler.
 * Parameters:  fd - STUN socket 
 *              event - the event flag for the STUN socket event
 *              arg - normally this is used to passed in an event struct in 
 *                    order to reschedule the event. It is not used here since 
 *                    the event is created with EV_PERSIST flag, i.e. no need
 *                    to reschedule in the callback routine.
 * Returns:     none
 * Side Effects: Relevant The STUN Server statistic counters are updated within
 *               the STUN message checking routine.
 */
static void stun_socket_read_callback (int fd, short event, void *arg)
{
    uint8_t msg_buffer[STUN_MSG_BUFFER_SIZE] = {0};
    int buf_size = sizeof(msg_buffer);
    struct iphdr *ip_header = NULL;
    struct udphdr *udp_header = NULL;
    int iphdrlen, udphdrlen;
    raw_packet_error_t ret = RAW_PACKET_OK;
    boolean stun_hdr_ok;

    ssize_t n;

    struct sockaddr_in from;
    socklen_t sock_len = sizeof(from);
    memset(&from, 0, sock_len);

    /* No need to reschedule this event anymore since we had set the 
     * EV_PERSIST flag when adding event for the first time.
     */

#if 0
    n = recvfrom(fd, msg_buffer, buf_size, 0, 
                 (struct sockaddr *)&from, &sock_len);
#endif

    int nocsum = 0;
    unsigned char cmsgbuf[CMSG_LEN(sizeof(struct tpacket_auxdata))];
    struct iovec iov = {
        .iov_base = msg_buffer,
        .iov_len = buf_size,
    };
    struct msghdr msg = {
        .msg_name = &from,
        .msg_namelen = sock_len,
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = cmsgbuf,
        .msg_controllen = sizeof(cmsgbuf),
    };
    struct cmsghdr *cmsg;
    memset(cmsgbuf, 0, sizeof cmsgbuf);

    n = recvmsg(fd, &msg, 0);

    if (n < 0) {
        /* recvfrom() error */
        if (errno != EAGAIN) {
            log_perror("recvfrom", errno);
            ss_global_stats.recvfrom_error++;
        }

        STUNSVR_FLTD_DEBUG(SS_DEBUG, from.sin_addr.s_addr,
                           "recvfrom() from %s:%d returns < 0: errno=%d\n",
                           inet_ntoa(from.sin_addr), ntohs(from.sin_port), 
                           errno);
        return;
    }

    /* */
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        
        STUNSVR_FLTD_DEBUG(SS_DEBUG, from.sin_addr.s_addr,
                           "cmsg->level=%d, cmsg->type=%d\n",
                           cmsg->cmsg_level, cmsg->cmsg_type);

        if (cmsg->cmsg_level == SOL_PACKET &&
            cmsg->cmsg_type == PACKET_AUXDATA) {
            struct tpacket_auxdata *aux = (void *)CMSG_DATA(cmsg);
            nocsum = aux->tp_status & TP_STATUS_CSUMNOTREADY;
        }
    }

    STUNSVR_FLTD_DEBUG(SS_DEBUG_VERBOSE, from.sin_addr.s_addr,
                       "Received from %s:%d: %d bytes\n",
                       inet_ntoa(from.sin_addr), ntohs(from.sin_port), n);

#ifdef DEBUG
    log_raw("Received STUN message", msg_buffer, n);
#endif

    /* Count the total number of the received request packets */
    ss_global_stats.total_req_rcvd++;

    /* All the packets received here have passed the STUN request packet 
     * filter, which means that the packets should be valid UDP packets (i.e.
     * have complete IP, UDP headers) and have valid STUN magic cookie.
     *
     * But as a defensive measure, do a sanity checking on the request frame
     * before calling the processing routine. Relevant counters are updated 
     * during the checking.
     */
    ip_header = (struct iphdr*)msg_buffer;
    udp_header = (struct udphdr*)(msg_buffer + ip_header->ihl*4);
    ret = packet_frame_check(msg_buffer, n);
    switch (ret) {
        case RAW_PACKET_OK:
            break;
        case RAW_PACKET_ERR_INVALIDARGS:
            STUNSVR_DEBUG(SS_DEBUG, NULL,
                          "Invalid args passed to stun_frame_checking\n");
            break;
        case RAW_PACKET_ERR_REQTOOSHORTIP:
            ss_global_stats.invalid_req_tooshort++;
            STUNSVR_DEBUG(SS_DEBUG, NULL, 
                          "Request packet is too short to have an IP header: "
                          "len=%d\n", n);
            break;
        case RAW_PACKET_ERR_REQTOOSHORTUDP:
            ss_global_stats.invalid_req_tooshort++;
            STUNSVR_DEBUG(SS_DEBUG, NULL, 
                   "too short to contain a UDP header (frame len=%d)", n);
            break;
        case RAW_PACKET_ERR_NOTUDP:
            ss_global_stats.invalid_req_notudp++;
            STUNSVR_DEBUG(SS_DEBUG, NULL, 
                     "not a UDP packet (proto=%d)\n", ip_header->protocol);
            break;
        case RAW_PACKET_ERR_UDPHEADERTOOLARGE:
            ss_global_stats.invalid_req_inconsistent++;
            STUNSVR_DEBUG(SS_DEBUG, NULL, 
                 "UDP header len %d is too large", ntohs(udp_header->len));
            break;
        case RAW_PACKET_ERR_INVALIDUDPCHKSUM:
            ss_global_stats.invalid_req_chksum++;
            STUNSVR_DEBUG(SS_DEBUG, NULL, 
                     "invalid UDP checksum 0x%x", udp_header->check);
            break;
    }

    /* Count the request packets that have IP options in their IP headers
     * Note everything should still work, it's just we are not going to check
     * its UDP checksum at the moment. We don't expect to see this counter
     * increment but if it turns out differently, we will have to take IP 
     * options into consideration while calculating UDP checksum.
     */
    if (ret != RAW_PACKET_ERR_REQTOOSHORTIP) {
        if (ip_header->ihl != 5) {
            ss_global_stats.req_has_ipoption++;
        }
    }

    /* Check if we have at least the fixed-length STUN hdr */
    iphdrlen = ip_header->ihl*4;
    udphdrlen = sizeof(struct udphdr);
    if ((n - iphdrlen - udphdrlen) < sizeof(stun_header_t)) {
        ss_global_stats.invalid_req_tooshort++;
        STUNSVR_DEBUG(SS_DEBUG, NULL, 
                      "too short to contain a STUN header (frame len=%d)", n);
        stun_hdr_ok = FALSE;
    } else {
        stun_hdr_ok = TRUE;
    }

    if (ret != RAW_PACKET_OK || !stun_hdr_ok) {
        STUNSVR_FLTD_DEBUG(SS_DEBUG, from.sin_addr.s_addr,
                           "Request packet from %s:%d failed "
                           "stun_frame_checking(): len=%d\n",
                           inet_ntoa(from.sin_addr), ntohs(from.sin_port), n);
        return;
    }

    /* The passed in buffer is large enough (1514 bytes) to hold the 
     * to-be-generated STUN response packet.  n is the length of the received
     * STUN request message, including the IP/UDP headers.
     */
    process_potential_stun_message(msg_buffer, n, fd);
}


int main (int argc, char *argv[])
{
    struct event evsocket;
    int raw_sock_fd = -1;
    raw_socket_error_t sock_err = RAW_SOCKET_OK;
    int sock_errno = 0;
    int retval;
    socklen_t optlen;
    vqe_cap_ret_t cap_ret;

    /* Initialize the logging facility */
    syslog_facility_open(LOG_STUN_SERVER, LOG_CONS);
    syslog_facility_filter_set(LOG_STUN_SERVER, LOG_WARNING);

    if (!event_lock_init()) {
        syslog_print(SS_INIT_FAILURE_CRIT,
                     "failed to initialize a global lock");
        goto failed;
    }

    struct event_mutex m = {&event_lock_lock,
                            &event_lock_unlock};

    /* Process command line option: exit when it returns -1 
     * (-h or -v options ) 
     */
    if (process_cmdline_option(argc, argv) == -1) {
        return (-1);
    }

    /* Set the log level again in case the level is changed from the command
     * line option.
     */
    syslog_facility_filter_set(LOG_STUN_SERVER, options.log_level);

    /* Drop privileges as soon as possible to avoid anything bad from
     * happening
     */
    cap_ret = vqe_keep_cap(options.uid, options.gid, VQE_CAP_CAPNETRAW_EIP);
    if (cap_ret != VQE_CAP_OK) {
        char syslog_buf[LOG_BUFFER_SIZE];
        snprintf(syslog_buf, LOG_BUFFER_SIZE, "capability drop failure: %s",
                 vqe_ret_tostr(cap_ret));
        syslog_print(SS_INIT_FAILURE_CRIT, syslog_buf);
        return(-1);
    }

    /* Enable debug logging if the log level is set to 7, LOG_DEBUG */
    if (options.log_level == LOG_DEBUG) {
        STUNSVR_SET_DEBUG_FLAG(SS_DEBUG);
        STUNSVR_SET_DEBUG_FLAG(SS_DEBUG_VERBOSE);
    }

    /* Log a debug message to show a few configuration options */
    STUNSVR_DEBUG(SS_DEBUG, NULL, "Configuration options: xmlrpc port=%d, "
                  "rcv_buflen=%d, snd_buflen=%d\n", 
                  options.xmlrpc_port, 
                  options.rcv_bufflen, options.snd_bufflen);

    /* Init statistic module */
    stunsvr_stats_init();

    /* Setup a non-blocking raw socket to receive STUN binding requests and
     * send out STUN binding responses. Attach the STUN request filter 
     * to the socket.
     */
    sock_err = create_raw_socket(IPPROTO_UDP,
                                 options.rcv_bufflen,
                                 options.snd_bufflen,
                                 &raw_sock_fd,
                                 &sock_errno);
    switch (sock_err) {
        case RAW_SOCKET_OK:
            break;
        case RAW_SOCKET_ERR_INVALIDARGS:
            STUNSVR_DEBUG(SS_DEBUG, NULL,
                          "invalid args passed to create_raw_socket\n");
            break;
        case RAW_SOCKET_ERR_SOCKET:
            log_perror("socket", sock_errno);
            break;
        case RAW_SOCKET_ERR_NONBLOCK:
            log_perror("fcntl O_NONBLOCK", sock_errno);
            break;
        case RAW_SOCKET_ERR_RCVBUF:
            log_perror("setsockopt SO_RCVBUF", sock_errno);
            break;
        case RAW_SOCKET_ERR_SNDBUF:
            log_perror("setsockopt SO_SNDBUF", sock_errno);
            break;
        case RAW_SOCKET_ERR_HDRINCL:
            log_perror("setsockopt IP_HDRINCL", sock_errno);
            break;
    }

    if (raw_sock_fd == -1) {
        goto failed;
    }

    /* To get tpacket_auxdata*/
    int val = 1;
    if (setsockopt(raw_sock_fd, SOL_PACKET, PACKET_AUXDATA, &val, sizeof val) 
        < 0) {
        if (errno != ENOPROTOOPT) {
            log_perror("setsockopt PACKET_AUXDATA", errno);
            goto failed;
        }
    }    

    /* Do a get on SO_RCVBUF and SO_SNDBUF to make sure they are set */
    optlen = sizeof(retval);
    if (getsockopt(raw_sock_fd, SOL_SOCKET, SO_RCVBUF, &retval, &optlen) 
        == 0) {
        STUNSVR_DEBUG(SS_DEBUG, NULL, 
                      "Socket receive buffer size set to %d\n", retval);
    }

    optlen = sizeof(retval);
    if (getsockopt(raw_sock_fd, SOL_SOCKET, SO_SNDBUF, &retval, &optlen) 
        == 0) {
        STUNSVR_DEBUG(SS_DEBUG, NULL, 
                      "Socket send buffer size set to %d\n", retval);
    }

    /* Attach filter */
    if (attach_stun_request_filter(raw_sock_fd) == -1) {
        syslog_print(SS_INIT_FAILURE_CRIT,
                     "failed to attach STUN filter to socket");
        goto failed;
    }

    /* Start the XMLRPC server thread */
    if (!stunsvr_xmlrpc_start(options.xmlrpc_port, options.xmlrpc_log)) {
        syslog_print(SS_INIT_FAILURE_CRIT,
                     "failed to start XMLRPC server thread");
        goto failed;
    }

    /* Initialize the event library */
    /*sa_ignore {do not need the event base with only one loop} IGNORE_RETURN*/
    event_init_with_lock(&m);

    event_set(&evsocket, raw_sock_fd, EV_READ | EV_PERSIST, 
              stun_socket_read_callback, &evsocket);

    /* Add it to the active events, without a timeout */
    if (event_add(&evsocket, NULL) == -1) {
        syslog_print(SS_INIT_FAILURE_CRIT,
                     "failed to add the libevent socket event");
        goto failed;
    }

    syslog_print(SS_STARTUP_COMPLETE_INFO);

    /* Entering libevent loop */
    (void)event_dispatch();

    close(raw_sock_fd);
    return 0;

failed:
    if (raw_sock_fd != -1) {
        close(raw_sock_fd);
    }
    return(-1);    
}
