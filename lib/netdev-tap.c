//
// Created by zhaogang on 2025-03-19.
//


#include <netdev/netdev.h>


int netdev_tap_run()
{
    return 0;
}

const struct netdev_class netdev_tap_class = {
        .type = "tap",
        .run = netdev_tap_run,
};