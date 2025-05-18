//
// Created by zhaogang on 2025-05-17.
//

#ifndef CVS_SERVER_ROUTER_H
#define CVS_SERVER_ROUTER_H
#include "cvs/hmap.h"

static struct HMap *RouterMap;

struct ServerRequest {
    char *data;
};
struct ServerResponse {
    char *data;
};

typedef struct ServerResponse (*handler_fn)(struct ServerRequest *);

int router_init();

int router_register(const char *path, handler_fn handler);

int router_lookup(const char *path, handler_fn *handler);


#endif //CVS_SERVER_ROUTER_H
