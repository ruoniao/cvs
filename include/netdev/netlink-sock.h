//
// Created by zhaogang on 2025-05-25.

/*
 * netlink 的Socket层
*/

#ifndef CVS_NETLINK_H
#define CVS_NETLINK_H

#include <stdbool.h>

struct nl_sock;

int nl_sock_create(int protocol, struct nl_sock **sock);
int nl_sock_destroy(struct nl_sock *sock);
int nl_sock_add_mcgroup(struct nl_sock *sock, unsigned int mcgroup);



int nl_sock_send(struct nl_sock *sock, const void *data, size_t len);
int nl_sock_recv(struct nl_sock *sock, void *buffer, size_t buffer_len,bool wait);


#endif //CVS_NETLINK_H
