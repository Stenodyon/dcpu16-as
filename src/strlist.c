/*
    This file is part of dcpu16-as.

    dcpu16-as is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    dcpu16-as is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with dcpu16-as.  If not, see <https://www.gnu.org/licenses/>.
*/

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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

void strlist_destroy(strlist_t *strlist)
{
    free(strlist->entries);
    free(strlist);
}
