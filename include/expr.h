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

#pragma once

#ifndef EXPR_H_
#define EXPR_H_

#include "hashmap.h"

#define EXPR_INT     1
#define EXPR_LABEL   2
#define EXPR_CURRENT 3 // Current byte in assembly

#define EXPR_ADD     4
#define EXPR_SUB     5
#define EXPR_MUL     6
#define EXPR_DIV     7
#define EXPR_MOD     8

typedef struct
{
    int nodetype;
} expr_t;

typedef struct
{
    int nodetype;
    int value;
} expr_int_t;

typedef struct
{
    int nodetype;
    char * name;
} expr_label_t;

typedef struct
{
    int nodetype;
    expr_t *lhs, *rhs;
} expr_binop_t;

expr_int_t * expr_int_make(int value);
expr_label_t * expr_label_make(char * label);
expr_t * expr_current_make(void);
expr_binop_t * expr_binop_make(int op, expr_t *lhs, expr_t *rhs);

void expr_fprint(FILE* file_handle, expr_t *expr);

void expr_eval_labels(expr_t **expr, hashmap_t *label_map);
void expr_eval_current(expr_t **expr, int current_byte);
void expr_simplify(expr_t **expr);
int expr_eval(expr_t *expr);

int expr_count_labels(expr_t *expr, int (*filter)(char *));

void expr_destroy(expr_t* expr);

#endif //EXPR_H_
