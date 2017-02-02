static int my_sendmsg(int fd, unsigned srcip, 
                      struct sockaddr *remote, char *data, int len)
{
    struct iovec iov = { data, len };
    struct {
        struct cmsghdr cm;
        struct in_pktinfo ipi;
    } cmsg = {
        .cm = {
            .cmsg_len = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo),
            .cmsg_level = SOL_IP,
            .cmsg_type = IP_PKTINFO,
        },
        .ipi = {
            .ipi_ifindex = 0,
            .ipi_spec_dst = srcip,
        },
    };

    struct msghdr m = {
        .msg_name = remote,
        .msg_namelen = sizeof(struct sockaddr_in),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = &cmsg,
        .msg_controllen = sizeof(cmsg),
        .msg_flags = 0,
    };

    return sendmsg(fd, &m, MSG_NOSIGNAL|MSG_DONTWAIT);
}

