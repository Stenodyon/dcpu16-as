#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "assemble.h"
#include "hashmap.h"
#include "expr.h"

typedef struct
{
    uint16_t location;
    expr_t * expr;
} valueref_t;

typedef struct
{
    int capacity;
    int size;
    valueref_t *refs;
} reflist_t;

static
void reflist_init(reflist_t *reflist)
{
    reflist->capacity = 8;
    reflist->size = 0;
    reflist->refs = (valueref_t*)malloc(8 * sizeof(valueref_t));
}

static
void reflist_insert(reflist_t *reflist, uint16_t location, expr_t *expr)
{
    if (reflist->size == reflist->capacity)
    {
        reflist->refs = (valueref_t*)realloc(reflist->refs,
                                             reflist->capacity * 2 * sizeof(valueref_t));
        reflist->capacity *= 2;
    }
    valueref_t *new_ref = &(reflist->refs[reflist->size++]);
    new_ref->location = location;
    new_ref->expr = expr;
}

static
valueref_t * reflist_get(reflist_t *reflist, int pos)
{
    return &(reflist->refs[pos]);
}

static
void reflist_clear(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
        expr_destroy(reflist->refs[i].expr);
    reflist->size = 0;
}

static
void reflist_destroy(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
        expr_destroy(reflist->refs[i].expr);
    free(reflist->refs);
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
    union _field
    {
        struct instr_field bits;
        uint16_t value;
    };
    union _field field;
    field.bits.opcode = instr->opcode;
    field.bits.a = instr->a->id;
    field.bits.b = instr->b->id;
    return field.value;
}

static
int is_local(char * label)
{
    return label[0] == '.'; // safe because \0 if empty string
}

static
int instr_size(struct ast_instr* instr)
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
    hashmap_t *label_map = hashmap_make();
    hashmap_t *local_label_map = hashmap_make();
    reflist_t reflist, local_reflist;
    reflist_init(&reflist);
    reflist_init(&local_reflist);

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
            buffer_append(buffer, assembled);
            if (verbose)
                printf("%04x: %04x", buffer->size - 1, assembled);
            if (instr->a->label_name)
            {
                if (is_local(instr->a->label_name))
                    reflist_insert(&local_reflist,
                                   buffer->size,
                                   (expr_t*)expr_label_make(instr->a->label_name));
                else
                    reflist_insert(&reflist,
                                   buffer->size,
                                   (expr_t*)expr_label_make(instr->a->label_name));
            }
            if (has_next_word(instr->a))
            {
                buffer_append(buffer, instr->a->nextword);
                if (verbose)
                    printf(" %04x", instr->a->nextword);
            }
            if (instr->b->label_name)
            {
                if (is_local(instr->b->label_name))
                    reflist_insert(&local_reflist,
                                   buffer->size,
                                   (expr_t*)expr_label_make(instr->b->label_name));
                else
                    reflist_insert(&reflist,
                                   buffer->size,
                                   (expr_t*)expr_label_make(instr->b->label_name));
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
        else if (stmt->nodetype == AST_LABEL)
        {
            struct ast_label* label = (struct ast_label*)stmt;
            if (is_local(label->name))
                hashmap_insert(local_label_map, label->name, buffer->size);
            else
            {
                for (int i = 0; i < local_reflist.size; i++)
                {
                    valueref_t *ref = reflist_get(&local_reflist, i);
                    expr_eval_labels(&(ref->expr), local_label_map);
                    int value = expr_eval(ref->expr);
                    buffer_set(buffer, ref->location, value);
                }
                reflist_clear(&local_reflist);
                hashmap_clear(local_label_map);
                hashmap_insert(label_map, label->name, buffer->size);
            }
        }
        else if (stmt->nodetype == AST_DATAW)
        {
            struct ast_dataw* dataw = (struct ast_dataw*)stmt;
            for (int i = 0; i < dataw->size; i++)
            {
                struct ast_dataw_val *dataval = &(dataw->data[i]);
                if (dataval->is_string)
                {
                    const char * str = (const char*)dataval->value;
                    int length = strlen(str);
                    for (int i = 0; i < length; i++)
                        buffer_append(buffer, str[i]);
                }
                else
                {
                    expr_t *expr = (expr_t*)dataval->value;
                    if (expr->nodetype == EXPR_INT)
                    {
                        expr_int_t *intexpr = (expr_int_t*)expr;
                        buffer_append(buffer, intexpr->value);
                    }
                    else
                    {
                        reflist_insert(&reflist, buffer->size, expr);
                        dataval->value = NULL;
                        buffer_append(buffer, 0);
                    }
                }
            }
        }
        else if (stmt->nodetype == AST_DATRS)
        {
            struct ast_datrs* datrs = (struct ast_datrs*)stmt;
            for (int i = 0; i < datrs->size; i++)
                buffer_append(buffer, 0x0000);
        }
    }

    // Set label locations at label references
    for (int i = 0; i < local_reflist.size; i++) // last local labels
    {
        valueref_t *ref = reflist_get(&local_reflist, i);
        expr_eval_labels(&(ref->expr), local_label_map);
        int value = expr_eval(ref->expr);
        buffer_set(buffer, ref->location, value);
    }
    for (int i = 0; i < reflist.size; i++) // global labels
    {
        valueref_t *ref = reflist_get(&reflist, i);
        expr_eval_labels(&(ref->expr), label_map);
        int value = expr_eval(ref->expr);
        buffer_set(buffer, ref->location, value);
    }

    hashmap_destroy(local_label_map);
    hashmap_destroy(label_map);
    reflist_destroy(&local_reflist);
    reflist_destroy(&reflist);
    return buffer;
}
