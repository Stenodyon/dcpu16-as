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
    operand_t* a, b;
};

operand_t* ast_make_operand(int id, uint16_t nextword);

struct ast_instr* ast_makeSET(operand_t* a, operand_t* b);

#endif //AST_H_
