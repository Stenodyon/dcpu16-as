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
    reflist->labelrefs = (labelref_t*)malloc(8 * sizeof(labelref_t));
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
labelref_t * reflist_get(reflist_t *reflist, int pos)
{
    return &(reflist->labelrefs[pos]);
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
                                   instr->a->label_name,
                                   buffer->size);
                else
                    reflist_insert(&reflist,
                                   instr->a->label_name,
                                   buffer->size);
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
                                   instr->b->label_name,
                                   buffer->size);
                else
                    reflist_insert(&reflist,
                                   instr->b->label_name,
                                   buffer->size);
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
                    labelref_t *ref = reflist_get(&local_reflist, i);
                    int location = hashmap_lookup(local_label_map, ref->name);
                    if (location == -1)
                    {
                        fprintf(stderr, "Undeclared label '%s'\n", ref->name);
                        exit(-1);
                    }
                    buffer_set(buffer, ref->location, location);
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
                buffer_append(buffer, dataw->data[i]);
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
        labelref_t *ref = reflist_get(&local_reflist, i);
        int location = hashmap_lookup(local_label_map, ref->name);
        if (location == -1)
        {
            fprintf(stderr, "Undeclared label '%s'\n", ref->name);
            exit(-1);
        }
        buffer_set(buffer, ref->location, location);
    }
    for (int i = 0; i < reflist.size; i++) // global labels
    {
        labelref_t *ref = reflist_get(&reflist, i);
        int location = hashmap_lookup(label_map, ref->name);
        if (location == -1)
        {
            fprintf(stderr, "Undeclared label '%s'\n", ref->name);
            exit(-1);
        }
        buffer_set(buffer, ref->location, location);
    }

    hashmap_destroy(label_map);
    reflist_destroy(&local_reflist);
    reflist_destroy(&reflist);
    return buffer;
}
