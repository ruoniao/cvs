//
// Created by zhaogang on 2025-03-19.
//

#include "netdev.h"
#include "cvs/hmap.h"

static struct HMap *netdev_classes;
#define HMAP_SIZE  16

/* 网卡初始化 */
static int netdev_init(){
    /* 初始化网卡设备 */
    netdev_classes = hmap_create(HMAP_SIZE);
    if (!netdev_classes) {
        fprintf(stderr, "Failed to create netdev classes hash map\n");
        return -1;
    }
    return 0;
}

/* 注册驱动 af_xdp tap dpdk */
static int netdev_register(const struct net_dev_class *netdev) {
    if (!netdev || !netdev->type) {
        fprintf(stderr, "Invalid netdev class\n");
        return -1;
    }

    /* 注册网卡设备 */
    hmap_put(netdev_classes, netdev->type, (void *)netdev);
    return 0;
}


int netdev_run(){
    /* 运行网卡设备 */
    struct HMapNode *node;
    for (size_t i = 0; i < netdev_classes->size; i++) {
        node = netdev_classes->buckets[i];
        while (node) {
            const struct net_dev_class *netdev = (const struct net_dev_class *)node->value;
            if (netdev && netdev->run) {
                netdev->run(netdev);
            }
            node = node->next;
        }
    }
    return 0;
}