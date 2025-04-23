//
// Created by zhaogang on 2025-03-19.
//
/*
 * 引入cvs/hmap.h 并实现其中的函数满足业务
 * */
// hmap.c
#include "cvs/hmap.h"
#include <stdlib.h>
#include <string.h>

#define HASH_MULTIPLIER 31

static size_t hash(const char *key, size_t size) {
    size_t hash = 0;
    while (*key) {
        hash = hash * HASH_MULTIPLIER + (unsigned char)(*key);
        key++;
    }
    return hash % size;
}

struct HMap *hmap_create(size_t size) {
    struct HMap *map = malloc(sizeof(struct HMap));
    if (!map) return NULL;
    map->buckets = calloc(size, sizeof(struct HMapNode *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    map->size = size;
    return map;
}

void hmap_put(struct HMap *map, const char *key, void *value) {
    size_t index = hash(key, map->size);
    struct HMapNode *node = map->buckets[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value;
            return;
        }
        node = node->next;
    }
    node = malloc(sizeof(struct HMapNode));
    if (!node) return;
    node->key = strdup(key);
    node->value = value;
    node->next = map->buckets[index];
    map->buckets[index] = node;
}

void *hmap_get(struct HMap *map, const char *key) {
    size_t index = hash(key, map->size);
    struct HMapNode *node = map->buckets[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void hmap_remove(struct HMap *map, const char *key) {
    size_t index = hash(key, map->size);
    struct HMapNode *node = map->buckets[index], *prev = NULL;
    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                map->buckets[index] = node->next;
            }
            free(node->key);
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void hmap_destroy(struct HMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        struct HMapNode *node = map->buckets[i];
        while (node) {
            struct HMapNode *next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }
    free(map->buckets);
    free(map);
}
