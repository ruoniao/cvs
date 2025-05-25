//
// Created by zhaogang on 2025-05-25.
//
/*
 * netlink 的netdev 层，对socket进行了处理，为上层netdev提供服务
*/


#ifndef CVS_NETLINK_NETDEV_H
#define CVS_NETLINK_NETDEV_H
#include "netdev/netlink-sock.h"
#include "linux/netlink.h"

struct rtnetlink_change {
    __u16 nlmsg_type;
    int ifindex;
    char ifname[IFNAMSIZ];
};


struct nl_sock *netdev_linux_notify_sock();

int rtnetlink_parse_msg(const void *buffer,struct rtnetlink_change *change);

#endif //CVS_NETLINK_NETDEV_H
