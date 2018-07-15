#pragma once

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdint.h>

typedef struct entry
{
    char * name;
    uint16_t location;
} entry_t;

typedef struct bucket
{
    int capacity;
    int size;
    entry_t * entries;
} bucket_t;

typedef struct hashmap
{
    int capacity;
    int size;
    bucket_t ** buckets;
} hashmap_t;

hashmap_t* hashmap_make();
void hashmap_insert(hashmap_t* hashmap, char * name, uint16_t location);
uint16_t hashmap_lookup(hashmap_t* hashmap, char * name);
void hashmap_destroy(hashmap_t* hashmap);

#endif //HASHMAP_H_
