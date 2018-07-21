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

void expr_eval_labels(expr_t **expr, hashmap_t *label_map);
void expr_eval_current(expr_t **expr, int current_byte);
uint16_t expr_eval(expr_t *expr);

void expr_destroy(expr_t* expr);

#endif //EXPR_H_
