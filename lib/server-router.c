//
// Created by zhaogang on 2025-05-17.
//
#include "server/server-router.h"
#include "cvs/hmap.h"
#define ROUTER_SIZE 2048

/* 路由处理抽象函数*/

int router_init() {
    // Initialize the router map
    RouterMap = hmap_create(ROUTER_SIZE);
    return 0;
}

int router_register(const char *path, void (*handler)(void)) {
    // Register a new route with the given path and handler
    hmap_put(RouterMap, path, handler);
    return 0;
}

int router_lookup(const char *path, void *handler) {
    // Lookup a route by path and return the associated handler
    void *result = hmap_get(RouterMap, path);
    if (result) {
        handler = result;
        return 0;
    }
    return -1; // Route not found
}