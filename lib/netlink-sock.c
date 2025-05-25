//
// Created by zhaogang on 2025-05-25.
//
#include <linux/netlink.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "unistd.h"
#include "netdev/netlink-sock.h"
#include "cvs/clog.h"


struct nl_sock {
    int fd; // 文件描述符
    int protocol; // 协议类型
};

int nl_sock_create(int protocol, struct nl_sock **sock) {
    if (!sock) {
        return -1; // 空指针
    }

    struct nl_sock *s = calloc(1, sizeof(struct nl_sock));
    if (!s) {
        return -1; // 内存分配失败
    }

    s->fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (s->fd < 0) {
        int err = errno;
        free(s);
        return -err; // 返回负 errno
    }

    struct sockaddr_nl addr = {
            .nl_family = AF_NETLINK,
            .nl_pid = getpid(), // 用户空间进程 PID
            .nl_groups = 0      // 默认不监听任何组，由上层 setsockopt 设置
    };

    if (bind(s->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        int err = errno;
        close(s->fd);
        free(s);
        return -err; // bind 失败
    }

    s->protocol = protocol;
    *sock = s;
    return 0; // 成功
}

int nl_sock_destroy(struct nl_sock *sock) {
    if (!sock) {
        return -1; // 错误：空指针
    }

    if (sock->fd >= 0) {
        close(sock->fd); // 关闭套接字
    }

    free(sock); // 释放内存
    return 0; // 成功
}

int nl_sock_recv(struct nl_sock *sock, void *buffer, size_t buffer_len, bool wait) {
    if (!sock || !buffer || buffer_len == 0) {
        return -1; // 错误：空指针或无效长度
    }

    struct sockaddr_nl nladdr;
    struct iovec iov = {
            .iov_base = buffer,
            .iov_len = buffer_len,
    };

    struct msghdr msg = {
            .msg_name = &nladdr,
            .msg_namelen = sizeof(nladdr),
            .msg_iov = &iov,
            .msg_iovlen = 1,
            .msg_control = NULL,
            .msg_controllen = 0,
            .msg_flags = 0,
    };
    ssize_t len = recvmsg(sock->fd, &msg, wait ? 0 : MSG_DONTWAIT);
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return errno; // 非阻塞模式下暂时无数据
        }
        return -1; // 接收失败
    }

    return 0;
}



int nl_sock_add_mcgroup(struct nl_sock *sock, unsigned int mcgroup) {
    if (!sock) {
        return -1; // 错误：空指针
    }

    if (setsockopt(sock->fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
                   &mcgroup, sizeof mcgroup) < 0) {
        // %u 无符号十进制整数
        LOG_ERROR("could not join multicast group %u (%s)",
                  mcgroup, errno);
        return errno;
    } else{
        LOG_INFO("Successfully joined multicast group %u", mcgroup);
    }

    return 0; // 成功添加多播组
}