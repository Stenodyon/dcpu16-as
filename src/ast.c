#include <stdlib.h>
#include <string.h>

#include "ast.h"

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

struct ast_label* ast_make_label(char* name)
{
    struct ast_label* label = (struct ast_label*)malloc(sizeof(struct ast_label));
    label->nodetype = AST_LABEL;
    label->name = name;
    return label;
}

struct ast_instr* ast_make_instr(int opcode, operand_t* a, operand_t* b)
{
    struct ast_instr* instr = (struct ast_instr*)malloc(sizeof(struct ast_instr));
    instr->nodetype = AST_INSTR;
    instr->opcode = opcode;
    instr->a = a;
    instr->b = b;
    return instr;
}

struct ast_dataw* ast_make_dataw()
{
    struct ast_dataw* dataw = (struct ast_dataw*)malloc(sizeof(struct ast_dataw));
    dataw->nodetype = AST_DATAW;
    dataw->capacity = 16;
    dataw->size = 0;
    dataw->data = (uint16_t*)malloc(16 * sizeof(uint16_t));
    return dataw;
}

struct ast_datrs* ast_make_datrs(int size)
{
    struct ast_datrs* datrs = (struct ast_datrs*)malloc(sizeof(struct ast_datrs));
    datrs->nodetype = AST_DATRS;
    datrs->size = size;
    return datrs;
}

void ast_destroy_stmt(struct ast_statement* stmt)
{
    if (stmt->nodetype == AST_LABEL)
    {
        struct ast_label* label = (struct ast_label*)stmt;
        free(label->name);
        free(label);
    }
    else if (stmt->nodetype == AST_INSTR)
    {
        struct ast_instr* instr = (struct ast_instr*)stmt;
        ast_destroy_operand(instr->a);
        ast_destroy_operand(instr->b);
        free(instr);
    }
    else if (stmt->nodetype == AST_DATAW)
    {
        struct ast_dataw* dataw = (struct ast_dataw*)stmt;
        free(dataw->data);
        free(dataw);
    }
    else if (stmt->nodetype == AST_DATRS)
    {
        free(stmt);
    }
}

static
void ast_dataw_insert(struct ast_dataw *dataw, uint16_t value)
{
    if (dataw->size == dataw->capacity)
    {
        dataw->data = (uint16_t*)realloc(dataw->data,
                dataw->capacity * 2 * sizeof(uint16_t));
        dataw->capacity *= 2;
    }
    dataw->data[dataw->size++] = value;
}

void ast_dataw_addint(struct ast_dataw *dataw, uint16_t value)
{
    ast_dataw_insert(dataw, value);
}

void ast_dataw_addstr(struct ast_dataw *dataw, const char * str)
{
    int length = strlen(str);
    for (int i = 0; i < length; i++)
    {
        uint16_t c = (uint16_t)str[i];
        ast_dataw_insert(dataw, c);
    }
}

ast_t* ast_make()
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
