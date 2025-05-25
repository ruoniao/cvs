//
// Created by zhaogang on 2025-05-25.
//
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include "netdev/netdev.h"
#include "cvs/clog.h"
#include "netdev/netlink-sock.h"
#include "netdev/netlink-netdev.h"
#include "cvs/thread.h"
#include "utils/check.h"


/* 创建nl socket 并添加监听事件*/
struct nl_sock *netdev_linux_notify_sock()
{
    static struct nl_sock *sock = NULL;
    static struct cvsthread_once once = CVS_THREAD_ONCE_INIT;
//    unsigned int mcgroups[] = {RTNLGRP_LINK, RTNLGRP_IPV4_IFADDR,
//                               RTNLGRP_IPV6_IFADDR, RTNLGRP_IPV6_IFINFO};
    unsigned int mcgroups[] = {RTNLGRP_LINK};
    if (cvs_thread_once_start(&once)) {
        nl_sock_create(NETLINK_ROUTE, &sock);
        if (!sock) {
            LOG_ERROR("Failed to create netlink socket for veth notifications");
            return NULL;
        }
        for (int i=0;i < ARRAY_SIZE(mcgroups); i++) {
            int ret = nl_sock_add_mcgroup(sock, mcgroups[i]);
            if (ret < 0) {
                LOG_ERROR("Failed to add multicast group %d to netlink socket: %d", mcgroups[i], ret);
                nl_sock_destroy(sock);
                return NULL;
            }
        }
        ovsthread_once_done(&once);
    }
    return sock;
}

/* 将netlink msg 解析为 rtnetlink_change 供netdev 使用*/
int rtnetlink_parse_msg(const void *buffer, struct rtnetlink_change *change) {
    if (!buffer || !change) {
        return -1; // 错误：空指针
    }

    const struct nlmsghdr *nlh = (const struct nlmsghdr *)buffer;
    if (nlh->nlmsg_len < sizeof(struct nlmsghdr)) {
        return -1; // 消息长度不够
    }

    change->nlmsg_type = nlh->nlmsg_type;

    // 仅处理 RTM_NEWLINK, RTM_DELLINK 类型的消息
    if (nlh->nlmsg_type == RTM_NEWLINK) {
        const struct ifinfomsg *ifi = (const struct ifinfomsg *)NLMSG_DATA(nlh);
        change->ifindex = ifi->ifi_index;
        // 解析附加属性
        int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
        struct rtattr *rta = IFLA_RTA(ifi);
        memset(change->ifname, 0, sizeof(change->ifname));
        for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            if (rta->rta_type == IFLA_IFNAME) {
                strncpy(change->ifname, (char *)RTA_DATA(rta), IFNAMSIZ - 1);
                change->ifname[IFNAMSIZ - 1] = '\0';
                break;
            }
        }
        // 判断网卡是否UP
        bool is_up = (ifi->ifi_flags & IFF_UP) != 0;
        change->is_up = is_up;
        bool is_running = (ifi->ifi_flags & IFF_RUNNING) != 0;
        change->is_running = is_running;
        return 0;
    }else if (nlh->nlmsg_type == RTM_DELLINK) {
        return 0; // 只处理删除链接消息
    }

    return 0; // 解析成功
}