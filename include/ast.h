#pragma once

#ifndef AST_H_
#define AST_H_

#include <stdint.h>

struct ast_operand
{
    int id;
    uint16_t nextword;
    char* label_name;
};

typedef struct ast_operand operand_t;

#define AST_LABEL 1
#define AST_INSTR 2

struct ast_statement
{
    int nodetype;
};

struct ast_label
{
    int nodetype;
    char* name;
};

struct ast_instr
{
    int nodetype;
    int opcode;
    operand_t *a, *b;
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

struct ast_label* ast_make_label(char * label);
struct ast_instr* ast_make_instr(int opcode, operand_t* a, operand_t* b);
struct ast_instr* ast_makeSET(operand_t* a, operand_t* b);
void ast_destroy_stmt(struct ast_statement* stmt);

ast_t* ast_make();
struct ast_statement* ast_get(ast_t* ast, int index);
void ast_append(ast_t* ast, struct ast_statement* stmt);
void ast_destroy(ast_t* ast);

#endif //AST_H_
