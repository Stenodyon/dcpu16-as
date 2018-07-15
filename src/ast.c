#include "ast.h"

operand_t * ast_make_operand(int id, uint16_t nextword)
{
    operand_t operand = (operand_t*)malloc(sizeof(operand_t));
    operand->id = id;
    operand->nextword = nextword;
    operand->label_name = NULL;
    return operand;
}

struct ast_instr* ast_makeSET(operand_t* a, operand_t* b)
{
    struct ast_instr* set_instr = (struct ast_instr*)malloc(sizeof(struct ast_instr));
    set_instr->nodetype = AST_INSTR;
    set_instr->opcode = 0x01;
    set_instr->a = a;
    set_instr->b = b;
    return set_instr;
}
