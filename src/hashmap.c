#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static bucket_t* bucket_make()
{
    bucket_t* bucket = (bucket_t*)malloc(sizeof(bucket_t));
    bucket->capacity = 1;
    bucket->size = 0;
    bucket->entries = (entry_t*)malloc(1 * sizeof(entry_t));
    return bucket;
}

static void bucket_insert(bucket_t* bucket, const char * name, uint16_t location)
{
    for (int i = 0; i < bucket->size; i++)
    {
        entry_t* entry = &(bucket->entries[i]);
        if (strcmp(name, entry->name) == 0)
        {
            entry->location = location;
            return;
        }
    }

    if (bucket->size == bucket->capacity)
    {
        bucket->entries = (entry_t*)realloc(bucket->entries,
                bucket->capacity * 2 * sizeof(entry_t));
        bucket->capacity *= 2;
    }
    entry_t * entry = &(bucket->entries[bucket->size++]);
    entry->name = name;
    entry->location = location;
}

static
int bucket_find(bucket_t* bucket, const char * name)
{
    for (int i = 0; i < bucket->size; i++)
    {
        entry_t* entry = &(bucket->entries[i]);
        if (strcmp(name, entry->name) == 0)
            return entry->location;
    }
    return -1;
}

static void bucket_destroy(bucket_t* bucket)
{
    free(bucket->entries);
    free(bucket);
}

static hashmap_t* hashmap_make_n(int capacity)
{
    hashmap_t* hashmap = (hashmap_t*)malloc(sizeof(hashmap_t));
    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->buckets = (bucket_t**)malloc(capacity * sizeof(bucket_t*));
    memset(hashmap->buckets, 0, capacity * sizeof(bucket_t*));
    return hashmap;
}

hashmap_t* hashmap_make()
{
    return hashmap_make_n(256);
}

static void hashmap_expand(hashmap_t* hashmap)
{
    hashmap_t* temp_hashmap = hashmap_make_n(hashmap->capacity * 2);

    for (int i = 0; i < hashmap->capacity; i++)
    {
        bucket_t* bucket = hashmap->buckets[i];
        if (!bucket)
            continue;
        for (int j = 0; j < bucket->size; j++)
        {
            entry_t* entry = &(bucket->entries[j]);
            hashmap_insert(temp_hashmap, entry->name, entry->location);
        }
    }
    hashmap->capacity *= 2;
    temp_hashmap->capacity /= 2;

    bucket_t** swap = hashmap->buckets;
    hashmap->buckets = temp_hashmap->buckets;
    temp_hashmap->buckets = swap;

    hashmap_destroy(temp_hashmap);
}

// djb2 by Dan Bernstein
static unsigned long hash(const unsigned char * str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

static inline
unsigned long hashmap_index(hashmap_t* hashmap, const char * name)
{
    return hash((const unsigned char *)name) % hashmap->capacity;
}

void hashmap_insert(hashmap_t* hashmap, const char * name, uint16_t location)
{
    if (hashmap->size >= hashmap->capacity * 2 / 3)
        hashmap_expand(hashmap);
    unsigned long index = hashmap_index(hashmap, name);
    bucket_t* bucket = hashmap->buckets[index];
    if (bucket == NULL)
    {
        bucket = bucket_make();
        hashmap->buckets[index] = bucket;
    }
    bucket_insert(bucket, name, location);
}

int hashmap_lookup(hashmap_t* hashmap, const char* name)
{
    unsigned long index = hashmap_index(hashmap, name);
    bucket_t* bucket = hashmap->buckets[index];
    if (bucket == NULL)
        return -1;
    return bucket_find(bucket, name);
}

void hashmap_destroy(hashmap_t* hashmap)
{
    for (int i = 0; i < hashmap->capacity; i++)
    {
        bucket_t* bucket = hashmap->buckets[i];
        if (!bucket)
            continue;
        bucket_destroy(bucket);
    }
    free(hashmap->buckets);
    free(hashmap);
}
