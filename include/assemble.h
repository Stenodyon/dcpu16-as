#pragma once

#ifndef ASSEMBLE_H_
#define ASSEMBLE_H_

#include <stdint.h>

#include "ast.h"

struct bin_buffer
{
    int capacity;
    int size;
    uint16_t * data;
};

struct instr_field
{
    uint16_t opcode : 5;
    uint16_t b : 5;
    uint16_t a : 6;
};

typedef struct bin_buffer bin_buffer_t;

bin_buffer_t* buffer_make();
void buffer_append(bin_buffer_t* buffer, uint16_t value);
void buffer_destroy(bin_buffer_t* buffer);

int has_next_word(operand_t* operand);
uint16_t assemble_instr(struct ast_instr* instr);
bin_buffer_t* assemble(ast_t* ast);

#endif //ASSEMBLE_H_
