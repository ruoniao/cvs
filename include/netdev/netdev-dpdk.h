//
// Created by zhaogang on 2025-03-19.
//

#ifndef CVS_NETDEV_DPDK_H
#define CVS_NETDEV_DPDK_H

#include <stdint.h>

/* cpuid */
typedef uint8_t lcoreid_t;
/* 端口id*/
typedef uint16_t portid_t;

/* netdev事件mask */
extern uint32_t event_print_mask;
#endif //CVS_NETDEV_DPDK_H
