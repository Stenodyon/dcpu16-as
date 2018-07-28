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
