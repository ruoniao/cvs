//
// Created by zhaogang on 2025-03-19.
//
/*
 * hash map 实现
 *
 * */

#ifndef CVS_HMAP_H
#define CVS_HMAP_H
#include <stddef.h>
#include <stdint.h>

struct hash_node {
    size_t hash;
    struct hash_node *next;
};

void hash_init(struct hash_node *hash, size_t size);

void hash_insert(struct hash_node *hash, size_t size, size_t key);

void hash_remove(struct hash_node *hash, size_t size, size_t key);

struct hash_node *hash_find(struct hash_node *hash, size_t size, size_t key);


#endif //CVS_HMAP_H
