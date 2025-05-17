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

typedef void *(*handler_fn)(const struct ServerRequest *, void *);

int router_init();

int router_register(const char *path, void (*handler)(void));

int router_lookup(const char *path, void *handler);


#endif //CVS_SERVER_ROUTER_H
