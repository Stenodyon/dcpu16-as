#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "assemble.h"
#include "hashmap.h"
#include "expr.h"
#include "ast.h"

static int is_local(char * label);

typedef struct
{
    ast_location_t source_location;
    uint16_t location;
    expr_t * expr;
} valueref_t;

typedef struct
{
    bin_buffer_t *assembly;
    int capacity;
    int size;
    valueref_t *refs;
} reflist_t;

static
void reflist_init(reflist_t *reflist, bin_buffer_t *assembly)
{
    reflist->assembly = assembly;
    reflist->capacity = 8;
    reflist->size = 0;
    reflist->refs = (valueref_t*)malloc(8 * sizeof(valueref_t));
}

// ----------------------------------------------------------------------------

static
void reflist_insert(reflist_t *reflist, ast_location_t source_location,
                    uint16_t location, expr_t *expr)
{
    if (reflist->size == reflist->capacity)
    {
        int new_capacity = 2 * reflist->capacity;
        reflist->refs = (valueref_t*)realloc(reflist->refs,
                                             new_capacity * sizeof(valueref_t));
        reflist->capacity = new_capacity;
    }
    valueref_t *new_ref = &(reflist->refs[reflist->size++]);
    new_ref->source_location = source_location;
    new_ref->location = location;
    new_ref->expr = expr;
}

// ----------------------------------------------------------------------------

static
valueref_t * reflist_get(reflist_t *reflist, int pos)
{
    return &(reflist->refs[pos]);
}

// ----------------------------------------------------------------------------

static
void reflist_eval_labels(reflist_t *reflist, hashmap_t *label_map)
{
    for (int i = 0; i < reflist->size; i++)
    {
        valueref_t *ref = reflist_get(reflist, i);
        expr_eval_labels(&(ref->expr), label_map);
    }
}

// ----------------------------------------------------------------------------

static
void reflist_clear(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
        expr_destroy(reflist->refs[i].expr);
    reflist->size = 0;
}

// ----------------------------------------------------------------------------

static
void reflist_transfer(reflist_t *dest, reflist_t *src)
{
    for (int i = 0; i < src->size; i++)
    {
        valueref_t *ref = reflist_get(src, i);
        reflist_insert(dest, ref->source_location, ref->location, ref->expr);
        ref->expr = NULL;
    }
    reflist_clear(src);
}

// ----------------------------------------------------------------------------

static
int reflist_find_locals(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
    {
        valueref_t *ref = reflist_get(reflist, i);
        if (expr_count_labels(ref->expr, is_local))
            return i;
    }
    return -1;
}

// ----------------------------------------------------------------------------

static
int reflist_has_locals(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
    {
        valueref_t *ref = reflist_get(reflist, i);
        if (expr_count_labels(ref->expr, is_local))
            return 1;
    }
    return 0;
}

// ----------------------------------------------------------------------------

static
void reflist_destroy(reflist_t *reflist)
{
    for (int i = 0; i < reflist->size; i++)
        expr_destroy(reflist->refs[i].expr);
    free(reflist->refs);
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

static
int is_local(char * label)
{
    return label[0] == '.'; // safe because \0 if empty string
}

// ----------------------------------------------------------------------------

#if 0
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
#endif

// ----------------------------------------------------------------------------

bin_buffer_t* assemble(ast_t* ast)
{
    bin_buffer_t* buffer = buffer_make();
    hashmap_t *label_map = hashmap_make();
    hashmap_t *local_label_map = hashmap_make();
    reflist_t reflist, local_reflist;
    reflist_init(&reflist, buffer);
    reflist_init(&local_reflist, buffer);

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
                printf("%04x: %04x", buffer->virtual_location - 1, assembled);
            if (instr->a->label_name)
            {
                reflist_t *label_reflist =
                    (is_local(instr->a->label_name))
                    ? &local_reflist
                    : &reflist;
                reflist_insert(label_reflist,
                               stmt->location,
                               buffer->virtual_location,
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
                reflist_t *label_reflist =
                    (is_local(instr->b->label_name))
                    ? &local_reflist
                    : &reflist;
                reflist_insert(label_reflist,
                               stmt->location,
                               buffer->virtual_location,
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
            {
                hashmap_insert(local_label_map,
                               label->name,
                               buffer->virtual_location);
            }
            else
            {
                reflist_eval_labels(&local_reflist, local_label_map);
                int index = reflist_find_locals(&local_reflist);
                if (index != -1)
                {
                    fprintf(stderr, "%s:%i Unresolved local label\n",
                            stmt->location.filename,
                            stmt->location.line);
                    exit(-1);
                }
                reflist_transfer(&reflist, &local_reflist);
                hashmap_clear(local_label_map);
                hashmap_insert(label_map,
                               label->name,
                               buffer->virtual_location);
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
                    expr_eval_current((expr_t**)&dataval->value,
                                      buffer->virtual_location);
                    expr_t *expr = (expr_t*)dataval->value;
                    if (expr->nodetype == EXPR_INT)
                    {
                        expr_int_t *intexpr = (expr_int_t*)expr;
                        buffer_append(buffer, intexpr->value);
                    }
                    else
                    {
                        reflist_insert(&reflist,
                                       stmt->location,
                                       buffer->virtual_location, expr);
                        dataval->value = NULL;
                        buffer_append(buffer, 0);
                    }
                }
            }
        }
        else if (stmt->nodetype == AST_DATRS)
        {
            struct ast_datrs* datrs = (struct ast_datrs*)stmt;
            expr_eval_current(&datrs->size_expr, buffer->virtual_location);
            int size = expr_eval(datrs->size_expr);
            if (size == -1)
            {
                fprintf(stderr, "%s:%i. could not evaluate expression ",
                        stmt->location.filename,
                        stmt->location.line);
                expr_fprint(stderr, datrs->size_expr);
                fprintf(stderr, "\n");
                exit(-1);
            }
            for (int i = 0; i < size; i++)
                buffer_append(buffer, 0x0000);
        }
    }

    reflist_eval_labels(&local_reflist, local_label_map);
    reflist_transfer(&reflist, &local_reflist);

    for (int i = 0; i < reflist.size; i++) // global labels
    {
        valueref_t *ref = reflist_get(&reflist, i);
        expr_eval_labels(&(ref->expr), label_map);
        int value = expr_eval(ref->expr);
        if (value == -1)
        {
            fprintf(stderr, "%s:%i. could not evaluate expression ",
                    ref->source_location.filename,
                    ref->source_location.line);
            expr_fprint(stderr, ref->expr);
            fprintf(stderr, "\n");
            exit(-1);
        }
        buffer_set(buffer, ref->location, value);
    }

    hashmap_destroy(local_label_map);
    hashmap_destroy(label_map);
    reflist_destroy(&local_reflist);
    reflist_destroy(&reflist);
    return buffer;
}
