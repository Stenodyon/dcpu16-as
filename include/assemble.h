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
