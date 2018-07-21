#pragma once

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdint.h>

typedef struct entry
{
    const char * name;
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

// Creates an emtpy hashmap. Caller is the owner of the returned pointer
hashmap_t* hashmap_make();

// Inserts an entry into the hashmap
void hashmap_insert(hashmap_t *hashmap, const char * name, uint16_t location);

// Returns the value indexed by `name`. Returns -1 if `name` is not in the map
int hashmap_lookup(hashmap_t *hashmap, const char * name);

// Removes all the elements from the hashmap
void hashmap_clear(hashmap_t *hashmap);

// Frees the memory of the hashmap
void hashmap_destroy(hashmap_t *hashmap);

#endif //HASHMAP_H_
