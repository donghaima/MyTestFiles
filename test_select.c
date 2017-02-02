/* Code snippet */

    fd_set rset;
    struct timeval tm;
    int nready, n;
    struct sockaddr_in from;
    socklen_t len = sizeof(struct sockaddr_in);

    /* Listening on the udp_sock_fd to receive the STUN response packet */
    tm.tv_sec = 1;
    tm.tv_usec = 0;

    FD_ZERO(&rset);
    FD_SET(udp_sock_fd, &rset);
again:
    nready = select(udp_sock_fd + 1, &rset, NULL, NULL, &tm);

    /* Failure if we don't receive anything after 1 second */
//    CU_ASSERT(nready != 0);
    printf("nready = %d\n", nready);

    printf("Stats: \n\tbinding_req_rcvd=%lld, binding_resp_failed_gen=%lld, "
           "binding_resp_sent_ok=%lld, binding_resp_send_failed=%lld\n",
           global_stats.binding_req_rcvd, 
           global_stats.binding_resp_failed_gen, 
           global_stats.binding_resp_sent_ok, 
           global_stats.binding_resp_send_failed);

    if (nready == -1) {
        if (errno == EINTR) {
            goto again;
        }
        fprintf(stderr, "select(): %m\n");

    } else if (nready) {
        /* udp_sock_fd receives data */
        if (FD_ISSET(udp_sock_fd, &rset)) {
            n = recvfrom(udp_sock_fd, msg_buffer, buf_size, 0, 
                         (struct sockaddr *)&from, &len);
            CU_ASSERT(nready > 0);
            
            printf("Received response: %d\n", n);
            
        }
        
    }


