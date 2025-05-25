//
// Created by zhaogang on 2025-03-19.
//

#include <linux/netlink.h>
#include <stdint.h>

#include "netdev/netdev.h"
#include "cvs/clog.h"
#include "netdev/netlink-sock.h"
#include "netdev/netlink-netdev.h"



void netdev_veth_init(const struct netdev_class *netdev)
{
    LOG_INFO("Init veth netdev class");
}

void netdev_veth_run(const struct netdev_class *netdev)
{
    int error;
    LOG_INFO("Running veth netdev class: %s", netdev->type);
    struct nl_sock *sock = netdev_linux_notify_sock();
    if (!sock) {
        LOG_ERROR("Failed to get netlink socket for veth notifications");
        return;
    }
    do {
        // uint64_t 跨平台一致
        uint64_t buffer[4096/8];
        // nl_sock_recv 返回错误码，正确返回0
        error = nl_sock_recv(sock, buffer, sizeof(buffer),true);
        if (!error) {
            // 处理接收到的 netlink 消息
            // 这里可以添加对 netlink 消息的具体处理逻辑
            // 1. 解析网卡状态变化
            struct rtnetlink_change change;
            error = rtnetlink_parse_msg(buffer, &change);
            if(!error){
                LOG_INFO("Netlink message type: %d, ifindex: %d, ifname: %s",
                         change.nlmsg_type, change.ifindex, change.ifname);
            } else{
                LOG_ERROR("Failed to parse netlink message: %d", error);
            }
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break; // 退出循环
        }
    } while (!error);

}


const struct netdev_class netdev_veth_class = {
        .type = "veth",
        .init = netdev_veth_init,
        .run  = netdev_veth_run,
};