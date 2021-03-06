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

#ifndef AST_H_
#define AST_H_

#include <stdint.h>

#include "expr.h"

struct ast_operand
{
    int id;
    uint16_t nextword;
    char* label_name;
};

typedef struct ast_operand operand_t;

#define AST_LABEL 1
#define AST_INSTR 2
#define AST_DATAW 3
#define AST_DATRS 4

typedef struct
{
    char * filename;
    int line;
} ast_location_t;

struct ast_statement
{
    int nodetype;
    ast_location_t location;
};

struct ast_label
{
    int nodetype;
    ast_location_t location;

    char* name;
};

struct ast_instr
{
    int nodetype;
    ast_location_t location;

    int opcode;
    operand_t *a, *b;
};

struct ast_dataw_val
{
    int is_string;
    void *value;
};

struct ast_dataw
{
    int nodetype;
    ast_location_t location;

    int capacity;
    int size;
    struct ast_dataw_val *data;
};

struct ast_datrs
{
    int nodetype;
    ast_location_t location;

    expr_t *size_expr;
};

struct ast_stmt_list
{
    int capacity;
    int size;
    struct ast_statement ** data;
};

typedef struct ast_stmt_list ast_t;

operand_t* ast_make_operand(int id, uint16_t nextword);
void ast_destroy_operand(operand_t * operand);

struct ast_label* ast_make_label(ast_location_t location, char * label);
struct ast_instr* ast_make_instr(ast_location_t location,
                                 int opcode, operand_t* a, operand_t* b);
struct ast_dataw* ast_make_dataw(ast_location_t location);
struct ast_datrs* ast_make_datrs(ast_location_t location, expr_t *size_expr);
void ast_destroy_stmt(struct ast_statement* stmt);

void ast_dataw_addint(struct ast_dataw *dataw, uint16_t value);
void ast_dataw_addstr(struct ast_dataw *dataw, const char * str);
void ast_dataw_addexp(struct ast_dataw *dataw, expr_t *expr);

ast_t* ast_make(void);
struct ast_statement* ast_get(ast_t* ast, int index);
void ast_append(ast_t* ast, struct ast_statement* stmt);
void ast_destroy(ast_t* ast);

#endif //AST_H_
