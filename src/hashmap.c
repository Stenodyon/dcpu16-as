#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

// Allocates a bucket
static
bucket_t* bucket_make()
{
    bucket_t* bucket = (bucket_t*)malloc(sizeof(bucket_t));
    bucket->capacity = 1;
    bucket->size = 0;
    bucket->entries = (entry_t*)malloc(1 * sizeof(entry_t));
    return bucket;
}

// ----------------------------------------------------------------------------

// Adds an entry to the bucket
static
void bucket_insert(bucket_t* bucket, const char * name, uint16_t location)
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

// ----------------------------------------------------------------------------

// Find the entry `name` in the bucket
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

// ----------------------------------------------------------------------------

// Removes all the elements from the bucket
static
void bucket_clear(bucket_t *bucket)
{
    bucket->size = 0;
}

// ----------------------------------------------------------------------------

// Frees the bucket
static
void bucket_destroy(bucket_t* bucket)
{
    free(bucket->entries);
    free(bucket);
}

// ----------------------------------------------------------------------------

static
hashmap_t* hashmap_make_n(int capacity)
{
    hashmap_t* hashmap = (hashmap_t*)malloc(sizeof(hashmap_t));
    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->buckets = (bucket_t**)malloc(capacity * sizeof(bucket_t*));
    memset(hashmap->buckets, 0, capacity * sizeof(bucket_t*));
    return hashmap;
}

// ----------------------------------------------------------------------------

hashmap_t* hashmap_make()
{
    return hashmap_make_n(256);
}

// ----------------------------------------------------------------------------

static
void hashmap_expand(hashmap_t* hashmap)
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

// ----------------------------------------------------------------------------

// djb2 by Dan Bernstein
static
unsigned long hash(const unsigned char * str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

// ----------------------------------------------------------------------------

static inline
unsigned long hashmap_index(hashmap_t* hashmap, const char * name)
{
    return hash((const unsigned char *)name) % hashmap->capacity;
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

int hashmap_lookup(hashmap_t* hashmap, const char* name)
{
    unsigned long index = hashmap_index(hashmap, name);
    bucket_t* bucket = hashmap->buckets[index];
    if (bucket == NULL)
        return -1;
    return bucket_find(bucket, name);
}

// ----------------------------------------------------------------------------

void hashmap_apply(hashmap_t *hashmap,
                   void (*fcn)(const char * key, uint16_t value))
{
    for (int bucket_id = 0; bucket_id < hashmap->capacity; bucket_id++)
    {
        bucket_t *bucket = hashmap->buckets[bucket_id];
        if (!bucket)
            continue;
        for (int entry_id = 0; entry_id < bucket->size; entry_id++)
        {
            entry_t *entry = &(bucket->entries[entry_id]);
            fcn(entry->name, entry->location);
        }
    }
}

// ----------------------------------------------------------------------------

void hashmap_clear(hashmap_t *hashmap)
{
    for (int i = 0; i < hashmap->capacity; i++)
    {
        bucket_t* bucket = hashmap->buckets[i];
        if (!bucket)
            continue;
        bucket_clear(bucket);
    }
    hashmap->size = 0;
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

int hashmap_iterator_is_end(hashmap_iterator_t *iterator)
{
    return iterator->_hashmap == NULL;
}

// ----------------------------------------------------------------------------

static
void hashmap_iterator_next_bucket(hashmap_iterator_t *iterator)
{
    hashmap_t * hashmap = iterator->_hashmap;
    if (!hashmap)
        return;

    iterator->_bucket_index++;
    if (iterator->_bucket_index >= hashmap->capacity)
    {
        iterator->_hashmap = NULL;
        return;
    }

    bucket_t *bucket;
    while (!(bucket = hashmap->buckets[iterator->_bucket_index]))
    {
        iterator->_bucket_index++;
        iterator->_entry_index = 0; // Reset for next bucket
        if (iterator->_bucket_index >= hashmap->capacity) // Reached the end
        {
            iterator->_hashmap = NULL;
            return;
        }
    }
}

// ----------------------------------------------------------------------------

hashmap_iterator_t hashmap_iter(hashmap_t *hashmap)
{
    hashmap_iterator_t iterator =
    {
        ._hashmap      = hashmap,
        ._bucket_index = 0,
        ._entry_index  = 0,
        .key           = NULL,
        .value         = 0
    };
    hashmap_iterator_next_bucket(&iterator);
    return iterator;
}

// ----------------------------------------------------------------------------

void hahsmap_iterator_next(hashmap_iterator_t *iterator)
{
    hashmap_t * hashmap = iterator->_hashmap;
    if (!hashmap)
        return;

    bucket_t *bucket = hashmap->buckets[iterator->_bucket_index];
    iterator->_entry_index++;
    while (iterator->_entry_index >= bucket->size)
    {
        hashmap_iterator_next_bucket(iterator);
        if (hashmap_iterator_is_end(iterator))
            return;
        bucket = hashmap->buckets[iterator->_bucket_index];
    }
    entry_t *entry = &(bucket->entries[iterator->_entry_index]);
    iterator->key = entry->name;
    iterator->value = entry->location;
}
