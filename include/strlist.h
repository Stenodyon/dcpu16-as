#pragma once

#ifndef STRLIST_H_
#define STRLIST_H_

typedef struct
{
    int size;
    int capacity;
    char ** entries;
} strlist_t;

strlist_t * strlist_make();
void strlist_insert(strlist_t * strlist, char * entry);
void strlist_destroy(strlist_t * strlist);

#endif //STRLIST_H_
