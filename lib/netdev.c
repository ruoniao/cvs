//
// Created by zhaogang on 2025-03-19.
//

#include "netdev/netdev.h"
#include "cvs/hmap.h"
#include "cvs/thread.h"

#define HMAP_SIZE  16
static struct HMap *netdev_classes = HMAP_INITIALIZER(HMAP_SIZE);


/* 网卡初始化 */
static int netdev_init(){
    /* 初始化网卡设备 通过线程锁执行下一次*/
    static struct cvsthread_once once = CVS_THREAD_ONCE_INIT;
    if (cvs_thread_once_start(&once)) {
        netdev_classes = hmap_create(HMAP_SIZE);
        if (!netdev_classes) {
            fprintf(stderr, "Failed to create netdev classes hash map\n");
            return -1;
        }
        struct HMapNode *node;
        for (size_t i = 0; i < netdev_classes->size; i++) {
            node = netdev_classes->buckets[i];
            while (node) {
                const struct netdev_class *netdev = (const struct netdev_class *)node->value;
                if (netdev && netdev->run) {
                    netdev->init();
                }
                node = node->next;
            }
        }
    }
    return 0;
}

/* 注册驱动 af_xdp tap dpdk */
static int netdev_register(const struct netdev_class *netdev) {
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
    netdev_init();
    struct HMapNode *node;
    for (size_t i = 0; i < netdev_classes->size; i++) {
        node = netdev_classes->buckets[i];
        while (node) {
            const struct netdev_class *netdev = (const struct netdev_class *)node->value;
            if (netdev && netdev->run) {
                netdev->run(netdev);
            }
            node = node->next;
        }
    }
    return 0;
}