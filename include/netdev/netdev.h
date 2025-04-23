//
// Created by zhaogang on 2025-03-19.
//
/*
 * 网卡设备的抽象
 * */

#ifndef CVS_NETDEV_H
#define CVS_NETDEV_H
#include <stdio.h>

/* 定义一个map 里面注册*/
struct net_dev_class {
    /* 网卡类型, tap af_xdp dpdk */
    const char *type;

/*## ------------ ##*/
/*## ---公共函数--- ##*/
/*## ----------- ##*/


    /* 初始化函数 */
    void (*init)(void );

    /* 运行函数 */
    void (*run)(const struct net_dev_class *netdev);

    /* 等待接收函数 */
    void (*wait)(const struct net_dev_class *netdev);

/*## ------------ ##*/
/*## ---网卡函数--- ##*/
/*## ----------- ##*/

    /* 发送函数 */
    void (*send)(const struct net_dev_class *netdev, const void *buf, size_t len);

    /* 接收函数 */
    void (*recv)(const struct net_dev_class *netdev, void *buf, size_t len);

    /* 关闭函数 */
    void (*close)(const struct net_dev_class *netdev);

};

/* 初始化网卡设备map */
static int netdev_init();

/* 注册驱动 af_xdp tap dpdk */
static int netdev_register(const struct net_dev_class *netdev);

/* 运行网卡设备 */
int netdev_run();




#endif //CVS_NETDEV_H
