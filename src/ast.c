#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "expr.h"

operand_t * ast_make_operand(int id, uint16_t nextword)
{
    operand_t* operand = (operand_t*)malloc(sizeof(operand_t));
    operand->id = id;
    operand->nextword = nextword;
    operand->label_name = NULL;
    return operand;
}

void ast_destroy_operand(operand_t* operand)
{
    if (operand->label_name)
        free(operand->label_name);
    free(operand);
}

struct ast_label* ast_make_label(ast_location_t location, char* name)
{
    struct ast_label* label = (struct ast_label*)malloc(sizeof(struct ast_label));
    label->nodetype = AST_LABEL;
    label->location = location;
    label->name = name;
    return label;
}

struct ast_instr* ast_make_instr(ast_location_t location,
                                 int opcode, operand_t* a, operand_t* b)
{
    struct ast_instr* instr = (struct ast_instr*)malloc(sizeof(struct ast_instr));
    instr->nodetype = AST_INSTR;
    instr->location = location;
    instr->opcode = opcode;
    instr->a = a;
    instr->b = b;
    return instr;
}

struct ast_dataw* ast_make_dataw(ast_location_t location)
{
    struct ast_dataw* dataw = (struct ast_dataw*)malloc(sizeof(struct ast_dataw));
    dataw->nodetype = AST_DATAW;
    dataw->location = location;
    dataw->capacity = 16;
    dataw->size = 0;
    dataw->data = (struct ast_dataw_val*)malloc(16 * sizeof(struct ast_dataw_val));
    return dataw;
}

struct ast_datrs* ast_make_datrs(ast_location_t location, expr_t *size_expr)
{
    struct ast_datrs* datrs = (struct ast_datrs*)malloc(sizeof(struct ast_datrs));
    datrs->nodetype = AST_DATRS;
    datrs->location = location;
    datrs->size_expr = size_expr;
    return datrs;
}

void ast_destroy_stmt(struct ast_statement* stmt)
{
    switch (stmt->nodetype)
    {
    case AST_LABEL:
    {
        struct ast_label* label = (struct ast_label*)stmt;
        free(label->name);
        free(label);
        break;
    }
    case AST_INSTR:
    {
        struct ast_instr* instr = (struct ast_instr*)stmt;
        ast_destroy_operand(instr->a);
        ast_destroy_operand(instr->b);
        free(instr);
        break;
    }
    case AST_DATAW:
    {
        struct ast_dataw* dataw = (struct ast_dataw*)stmt;
        for (int i = 0; i < dataw->size; i++)
        {
            struct ast_dataw_val *dataval = &(dataw->data[i]);
            if (dataval->is_string)
                free(dataval->value);
            else if(dataval->value)
                expr_destroy((expr_t*)(dataval->value));
        }
        free(dataw->data);
        free(dataw);
        break;
    }
    case AST_DATRS:
        free(stmt);
        break;
    default:
        fprintf(stderr, "Error while destroying AST: "
                "Unknown node type %i\n", stmt->nodetype);
        exit(-1);
    }
}

static
void ast_dataw_insert(struct ast_dataw *dataw, struct ast_dataw_val *value)
{
    if (dataw->size == dataw->capacity)
    {
        dataw->data = (struct ast_dataw_val*)realloc(dataw->data,
                      dataw->capacity * 2 * sizeof(struct ast_dataw_val));
        dataw->capacity *= 2;
    }
    dataw->data[dataw->size++] = *value;
}

void ast_dataw_addint(struct ast_dataw *dataw, uint16_t value)
{
    struct ast_dataw_val intval;
    intval.is_string = 0;
    intval.value = (void*)expr_int_make(value);
    ast_dataw_insert(dataw, &intval);
}

void ast_dataw_addstr(struct ast_dataw *dataw, const char * str)
{
    struct ast_dataw_val strval;
    strval.is_string = 1;
    strval.value = (void*)str;
    ast_dataw_insert(dataw, &strval);
}

void ast_dataw_addexp(struct ast_dataw *dataw, expr_t *expr)
{
    struct ast_dataw_val expval;
    expval.is_string = 0;
    expval.value = (void*)expr;
    ast_dataw_insert(dataw, &expval);
}

ast_t* ast_make(void)
{
    ast_t* ast = (ast_t*)malloc(sizeof(ast_t));
    ast->capacity = 256;
    ast->size = 0;
    ast->data = (struct ast_statement**)malloc(256 * sizeof(struct ast_statement*));
    return ast;
}

struct ast_statement* ast_get(ast_t* ast, int index)
{
    return ast->data[index];
}

void ast_append(ast_t* ast, struct ast_statement* stmt)
{
    if (ast->size == ast->capacity)
    {
        ast->data = (struct ast_statement**)realloc(ast->data,
                    ast->capacity * 2 * sizeof(struct ast_statement*));
        ast->capacity *= 2;
    }
    ast->data[ast->size++] = stmt;
}

void ast_destroy(ast_t* ast)
{
    for (int i = 0; i < ast->size; i++)
        ast_destroy_stmt(ast->data[i]);
    free(ast->data);
    free(ast);
}
