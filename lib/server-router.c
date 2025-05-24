//
// Created by zhaogang on 2025-05-17.
//
#include "server/server-router.h"
#include "cvs/hmap.h"
#include "utils/check.h"
#include "cvs/clog.h"


#define ROUTER_SIZE 2048

/* 路由处理抽象函数*/


struct ServerResponse handler_health_check(struct ServerRequest *request) {
    // Perform health check logic here
    // For example, check if the server is running, database is accessible, etc.
    LOG_DEBUG("Health check passed\n");
    struct ServerResponse response = {
            .data = "OK\n"
    };
    return response;

}


int router_init() {
    // Initialize the router map
    RouterMap = hmap_create(ROUTER_SIZE);
    router_register("/api/v1/health", &handler_health_check);
    return 0;
}

int router_register(const char *path, handler_fn handler) {
    // Register a new route with the given path and handler
    hmap_put(RouterMap, path, handler);
    return 0;
}

int router_lookup(const char *path, handler_fn *handler) {
    // Lookup a route by path and return the associated handler
    void *result = hmap_get(RouterMap, path);
    if (result) {
        *handler = (handler_fn)result;
        return 1;
    }
    return 0; // Route not found
}