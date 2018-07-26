#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "expr.h"

expr_int_t * expr_int_make(int value)
{
    expr_int_t *expr = (expr_int_t*)malloc(sizeof(expr_int_t));
    expr->nodetype = EXPR_INT;
    expr->value = value;
    return expr;
}

// ----------------------------------------------------------------------------

expr_label_t * expr_label_make(char * label)
{
    expr_label_t *expr = (expr_label_t*)malloc(sizeof(expr_label_t));
    expr->nodetype = EXPR_LABEL;
    expr->name = strdup(label);
    return expr;
}

// ----------------------------------------------------------------------------

expr_t * expr_current_make(void)
{
    expr_t *expr = (expr_t*)malloc(sizeof(expr_t));
    expr->nodetype = EXPR_CURRENT;
    return expr;
}

// ----------------------------------------------------------------------------

expr_binop_t * expr_binop_make(int op, expr_t *lhs, expr_t *rhs)
{
    expr_binop_t *expr = (expr_binop_t*)malloc(sizeof(expr_binop_t));
    expr->nodetype = op;
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}

// ----------------------------------------------------------------------------

void expr_fprint(FILE* file_handle, expr_t *expr)
{
    if (expr == NULL)
    {
        printf("NULL_POINTER");
        return;
    }
    switch (expr->nodetype)
    {
    case EXPR_INT:
        fprintf(file_handle, "%i", ((expr_int_t*)expr)->value);
        break;
    case EXPR_CURRENT:
        fprintf(file_handle, "$");
        break;
    case EXPR_LABEL:
        fprintf(file_handle, "%s", ((expr_label_t*)expr)->name);
        break;
    case EXPR_ADD:
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->lhs);
        fprintf(file_handle, ")");
        fprintf(file_handle, " + ");
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->rhs);
        fprintf(file_handle, ")");
        break;
    case EXPR_SUB:
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->lhs);
        fprintf(file_handle, ")");
        fprintf(file_handle, " - ");
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->rhs);
        fprintf(file_handle, ")");
        break;
    case EXPR_MUL:
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->lhs);
        fprintf(file_handle, ")");
        fprintf(file_handle, " * ");
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->rhs);
        fprintf(file_handle, ")");
        break;
    case EXPR_DIV:
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->lhs);
        fprintf(file_handle, ")");
        fprintf(file_handle, " / ");
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->rhs);
        fprintf(file_handle, ")");
        break;
    case EXPR_MOD:
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->lhs);
        fprintf(file_handle, ")");
        fprintf(file_handle, " %% ");
        fprintf(file_handle, "(");
        expr_fprint(file_handle, ((expr_binop_t*)expr)->rhs);
        fprintf(file_handle, ")");
        break;
    }
}

// ----------------------------------------------------------------------------

static
void _expr_eval_labels(expr_t **expr, hashmap_t *label_map)
{
    switch ((*expr)->nodetype)
    {
    case EXPR_LABEL:
    {
        expr_label_t *label_expr = (expr_label_t*)(*expr);
        int location = hashmap_lookup(label_map, label_expr->name);
        if (location == -1)
            return;
        expr_destroy(*expr);
        *expr = (expr_t*)expr_int_make(location);
    }
    break;
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    {
        expr_binop_t* binop = (expr_binop_t*)(*expr);
        _expr_eval_labels(&(binop->rhs), label_map);
        _expr_eval_labels(&(binop->lhs), label_map);
    }
    break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------

void expr_eval_labels(expr_t **expr, hashmap_t *label_map)
{
    _expr_eval_labels(expr, label_map);
    expr_simplify(expr);
}

// ----------------------------------------------------------------------------

static
void _expr_eval_current(expr_t **expr, int current_byte)
{
    switch ((*expr)->nodetype)
    {
    case EXPR_CURRENT:
        expr_destroy(*expr);
        *expr = (expr_t*)expr_int_make(current_byte);
        break;
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    {
        expr_binop_t* binop = (expr_binop_t*)(*expr);
        _expr_eval_current(&(binop->rhs), current_byte);
        _expr_eval_current(&(binop->lhs), current_byte);
    }
    break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------

void expr_eval_current(expr_t **expr, int current_byte)
{
    _expr_eval_current(expr, current_byte);
    expr_simplify(expr);
}

// ----------------------------------------------------------------------------

void expr_simplify(expr_t **expr)
{
    switch ((*expr)->nodetype)
    {
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    {
        expr_binop_t* binop = (expr_binop_t*)(*expr);
        expr_simplify(&(binop->lhs));
        expr_simplify(&(binop->rhs));
        if (binop->lhs->nodetype == EXPR_INT
                && binop->rhs->nodetype == EXPR_INT)
        {
            int lhs = ((expr_int_t*)binop->lhs)->value;
            int rhs = ((expr_int_t*)binop->rhs)->value;
            switch ((*expr)->nodetype)
            {
            case EXPR_ADD:
                *expr = (expr_t*)expr_int_make(lhs + rhs);
                break;
            case EXPR_SUB:
                *expr = (expr_t*)expr_int_make(lhs - rhs);
                break;
            case EXPR_MUL:
                *expr = (expr_t*)expr_int_make(lhs * rhs);
                break;
            case EXPR_DIV:
                if (rhs == 0)
                {
                    fprintf(stderr, "Error, division by zero\n");
                    exit(-1);
                }
                *expr = (expr_t*)expr_int_make(lhs / rhs);
                break;
            case EXPR_MOD:
                *expr = (expr_t*)expr_int_make(lhs % rhs);
                break;
            }
        }
        break;
    }
    default:
        break;
    }
}

// ----------------------------------------------------------------------------

int expr_eval(expr_t *expr)
{
    expr_t *e = expr;
    expr_simplify(&e);
    if (e->nodetype != EXPR_INT)
        return -1;
    return ((expr_int_t*)e)->value;
}

// ----------------------------------------------------------------------------

int expr_count_labels(expr_t *expr, int (*filter)(char *))
{
    if (expr == NULL)
        return 0;

    switch (expr->nodetype)
    {
    case EXPR_LABEL:
        if (filter(((expr_label_t*)expr)->name))
            return 1;
        return 0;
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
        {
            int lhs = expr_count_labels(((expr_binop_t*)expr)->lhs, filter);
            int rhs = expr_count_labels(((expr_binop_t*)expr)->rhs, filter);
            return lhs + rhs;
            break;
        }
    default:
        break;
    }
    return 0;
}

// ----------------------------------------------------------------------------

void expr_destroy(expr_t* expr)
{
    if (expr == NULL)
        return;

    switch (expr->nodetype)
    {
    case EXPR_LABEL:
    {
        expr_label_t* label = (expr_label_t*)expr;
        free(label->name);
        break;
    }
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    {
        expr_binop_t* binop = (expr_binop_t*)expr;
        expr_destroy(binop->lhs);
        expr_destroy(binop->rhs);
        break;
    }
    default:
        break;
    }
    free(expr);
}
