#include <stdlib.h>
#include <stdio.h>

#include "assemble.h"
#include "hashmap.h"
#include "config.h"

typedef struct
{
    char * name;
    uint16_t location;
} labelref_t;

typedef struct
{
    int capacity;
    int size;
    labelref_t *labelrefs;
} reflist_t;

static
void reflist_init(reflist_t *reflist)
{
    reflist->capacity = 8;
    reflist->size = 0;
    reflist->labelrefs = (labelref_t*)malloc(8 * sizeof(labelrefs));
}

static
void reflist_insert(reflist_t *reflist, char * label_name, uint16_t location)
{
    if (reflist->size == reflist->capacity)
    {
        reflist->labelrefs = (labelref_t*)realloc(reflist->labelrefs,
                reflist->capacity * 2 * sizeof(labelref_t));
        reflist->capacity *= 2;
    }
    labelref_t *new_ref = &(reflist->labelrefs[reflist->size++]);
    new_ref->name = label_name;
    new_ref->location = location;
}

static
void reflist_clear(reflist_t *reflist)
{
    reflist->size = 0;
}

static
void reflist_destroy(reflist_t *reflist)
{
    free(reflist->labelrefs);
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

static int instr_size(struct ast_instr* instr)
{
    int size = 1;
    if (instr->opcode != 0 && has_next_word(instr->b))
        size++;
    if (has_next_word(instr->a))
        size++;
    return size;
}

bin_buffer_t* assemble(ast_t* ast)
{
    bin_buffer_t* buffer = buffer_make();

    hashmap_t* label_map = hashmap_make();

    uint16_t current_word = 0;
    for (int i = 0; i < ast->size; i++)
    {
        struct ast_statement* stmt = ast_get(ast, i);
        if (stmt->nodetype == AST_INSTR)
        {
            struct ast_instr* instr = (struct ast_instr*)stmt;
            current_word += instr_size(instr);
        }
        else if (stmt->nodetype == AST_LABEL)
        {
            struct ast_label* label = (struct ast_label*)stmt;
            hashmap_insert(label_map, label->name, current_word);
            if (verbose)
                printf("label '%s' -> %04x\n", label->name, current_word);
        }
        else if (stmt->nodetype == AST_DATAW)
        {
            struct ast_dataw *dataw = (struct ast_dataw*)stmt;
            current_word += dataw->size;
        }
        else if (stmt->nodetype == AST_DATRS)
        {
            struct ast_datrs* datrs = (struct ast_datrs*)stmt;
            current_word += datrs->size;
        }
    }

    for (int i = 0; i < ast->size; i++)
    {
        struct ast_statement* stmt = ast_get(ast, i);
        if (stmt->nodetype == AST_INSTR)
        {
            struct ast_instr* instr = (struct ast_instr*)stmt;
            uint16_t assembled = assemble_instr(instr);
#ifdef _DEBUG
            printf("opcode: %02x, a: %02x, b: %02x -> %04x\n",
                    instr->opcode,
                    instr->a->id,
                    instr->b->id,
                    assembled);
#endif
            if (instr->a->label_name)
            {
                int label_location = hashmap_lookup(label_map, instr->a->label_name);
                if (label_location == -1)
                {
                    fprintf(stderr, "Unknown label '%s'\n", instr->a->label_name);
                    exit(-1);
                }
                instr->a->nextword = label_location;
            }
            if (instr->b->label_name)
            {
                int label_location = hashmap_lookup(label_map, instr->b->label_name);
                if (label_location == -1)
                {
                    fprintf(stderr, "Unknown label '%s'\n", instr->b->label_name);
                    exit(-1);
                }
                instr->b->nextword = label_location;
            }
            buffer_append(buffer, assembled);
            if (verbose)
                printf("%04x: %04x", buffer->size - 1, assembled);
            if (has_next_word(instr->a))
            {
                buffer_append(buffer, instr->a->nextword);
                if (verbose)
                    printf(" %04x", instr->a->nextword);
            }
            if (instr->opcode != 0 && has_next_word(instr->b))
            {
                buffer_append(buffer, instr->b->nextword);
                if (verbose)
                    printf(" %04x", instr->b->nextword);
            }
            if (verbose)
                printf("\n");
        }
        else if (stmt->nodetype == AST_DATAW)
        {
            struct ast_dataw* dataw = (struct ast_dataw*)stmt;
            for (int i = 0; i < dataw->size; i++)
                buffer_append(buffer, dataw->data[i]);
        }
        else if (stmt->nodetype == AST_DATRS)
        {
            struct ast_datrs* datrs = (struct ast_datrs*)stmt;
            for (int i = 0; i < datrs->size; i++)
                buffer_append(buffer, 0x0000);
        }
    }

    hashmap_destroy(label_map);
    return buffer;
}
