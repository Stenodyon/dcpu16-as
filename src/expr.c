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

expr_label_t * expr_label_make(char * label)
{
    expr_label_t *expr = (expr_label_t*)malloc(sizeof(expr_label_t));
    expr->nodetype = EXPR_LABEL;
    expr->name = strdup(label);
    return expr;
}

expr_t * expr_current_make(void)
{
    expr_t *expr = (expr_t*)malloc(sizeof(expr_t));
    expr->nodetype = EXPR_CURRENT;
    return expr;
}

expr_binop_t * expr_binop_make(int op, expr_t *lhs, expr_t *rhs)
{
    expr_binop_t *expr = (expr_binop_t*)malloc(sizeof(expr_binop_t));
    expr->nodetype = op;
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}

void expr_eval_labels(expr_t **expr, hashmap_t *label_map)
{
    switch ((*expr)->nodetype)
    {
    case EXPR_LABEL:
    {
        expr_label_t *label_expr = (expr_label_t*)(*expr);
        int location = hashmap_lookup(label_map, label_expr->name);
        if (location == -1)
        {
            fprintf(stderr, "Undeclared label '%s'\n", label_expr->name);
            exit(-1);
        }
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
        expr_eval_labels(&(binop->rhs), label_map);
        expr_eval_labels(&(binop->lhs), label_map);
    }
    break;
    default:
        break;
    }
}

void expr_eval_current(expr_t **expr, int current_byte)
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
        expr_eval_current(&(binop->rhs), current_byte);
        expr_eval_current(&(binop->lhs), current_byte);
    }
    break;
    default:
        break;
    }
}

uint16_t expr_eval(expr_t *expr)
{
    switch (expr->nodetype)
    {
    case EXPR_INT:
    {
        expr_int_t *expr_int = (expr_int_t*)expr;
        return expr_int->value;
    }
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    {
        expr_binop_t* binop = (expr_binop_t*)expr;
        int lhs = expr_eval(binop->lhs);
        int rhs = expr_eval(binop->rhs);
        switch (expr->nodetype)
        {
        case EXPR_ADD:
            return lhs + rhs;
        case EXPR_SUB:
            return lhs - rhs;
        case EXPR_MUL:
            return lhs * rhs;
        case EXPR_DIV:
            if (rhs == 0)
            {
                fprintf(stderr, "Error, division by zero\n");
                exit(-1);
            }
            return lhs / rhs;
        case EXPR_MOD:
            return lhs % rhs;
        }
    }
    break;
    default:
        fprintf(stderr, "Error, could not evaluate expression");
        exit(-1);
    }
}

void expr_destroy(expr_t* expr)
{
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
