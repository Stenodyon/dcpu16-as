#include "stdlib.h"

#include "strlist.h"

strlist_t * strlist_make()
{
    strlist_t *strlist = (strlist_t*)malloc(sizeof(strlist_t));
    strlist->size = 0;
    strlist->capacity = 10;
    strlist->entries = (char**)malloc(10 * sizeof(char*));
    return strlist;
}

void strlist_insert(strlist_t *strlist, char *entry)
{
    if (strlist->size == strlist->capacity)
    {
        strlist->entries = (char**)realloc(strlist->entries,
                strlist->capacity * 2 * sizeof(char*));
        strlist->capacity *= 2;
    }
    strlist->entries[strlist->size++] = entry;
}

void strlist_destroy(strlist_t *strlist)
{
    free(strlist->entries);
    free(strlist);
}
