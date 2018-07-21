#pragma once

#ifndef ASSEMBLE_H_
#define ASSEMBLE_H_

#include <stdint.h>

#include "ast.h"
#include "buffer.h"

struct instr_field
{
    uint16_t opcode : 5;
    uint16_t b : 5;
    uint16_t a : 6;
};

int has_next_word(operand_t *operand);
uint16_t assemble_instr(struct ast_instr *instr);
bin_buffer_t* assemble(ast_t *ast);

#endif //ASSEMBLE_H_
