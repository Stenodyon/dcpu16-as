#include <stdlib.h>
#include <string.h>
#include "utils.h"

char * strdup(char * str)
{
    int size = strlen(str) + 1;
    char * copy = (char*)malloc(size * sizeof(char));
    memcpy(copy, str, size);
    return copy;
}
