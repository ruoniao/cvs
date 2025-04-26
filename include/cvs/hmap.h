//
// Created by zhaogang on 2025-03-19.
//
/*
 * hash map 实现
 *
 * */
#ifndef HMAP_H
#define HMAP_H

#include <stddef.h>

struct HMapNode {
    char *key;
    void *value;
    struct HMapNode *next;
};

struct HMap{
    struct HMapNode **buckets;
    size_t size;
};

#define HMAP_INITIALIZER(HMAP_SIZE) &(struct HMap){\
    .buckets = NULL, \
    .size = HMAP_SIZE, \
}

struct HMap *hmap_create(size_t size);
void *hmap_put(struct HMap *map, const char *key, void *value);
void *hmap_get(struct HMap *map, const char *key);
void *hmap_remove(struct HMap *map, const char *key);
void *hmap_destroy(struct HMap *map);

#endif //CVS_HMAP_H

