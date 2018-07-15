#include <stdlib.h>

#include "assemble.h"

bin_buffer_t* buffer_make()
{
    bin_buffer_t* buffer = (bin_buffer_t*)malloc(sizeof(bin_buffer_t));
    buffer->capacity = 256;
    buffer->size = 0;
    buffer->data = (uint16_t*)malloc(256 * sizeof(uint16_t));
    return buffer;
}

void buffer_append(bin_buffer_t* buffer, uint16_t value)
{
    if (buffer->size == buffer->capacity)
    {
        buffer->data = (uint16_t*)realloc(buffer->data,
                buffer->capacity * 2 * sizeof(uint16_t));
        buffer->capacity *= 2;
    }
    buffer->data[buffer->size++] = value;
}

void buffer_destroy(bin_buffer_t* buffer)
{
    free(buffer->data);
    free(buffer);
}

int has_next_word(operand_t* operand)
{
    if (operand->id >= 0x10 && operand->id <= 0x17)
        return 1;
    if (operand->id == 0x1A)
        return 1;
    if (operand->id == 0x1E || operand->id == 0x1F)
        return 1;

    return 0;
}

uint16_t assemble_instr(struct ast_instr* instr)
{
    union _field {
        struct instr_field bits;
        uint16_t value;
    };
    union _field field;
    field.bits.opcode = instr->opcode;
    field.bits.a = instr->a->id;
    field.bits.b = instr->b->id;
    return field.value;
}

bin_buffer_t* assemble(ast_t* ast)
{
    bin_buffer_t* buffer = buffer_make();

    for (int i = 0; i < ast->size; i++)
    {
        struct ast_statement* stmt = ast_get(ast, i);
        if (stmt->nodetype == AST_INSTR)
        {
            struct ast_instr* instr = (struct ast_instr*)stmt;
            buffer_append(buffer, assemble_instr(instr));
            if (has_next_word(instr->a))
                buffer_append(buffer, instr->a->nextword);
            if (has_next_word(instr->b))
                buffer_append(buffer, instr->a->nextword);
        }
    }

    return buffer;
}
