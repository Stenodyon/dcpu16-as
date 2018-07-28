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

#include <check.h>

#include "expr_tests.h"
#include "hashmap.h"
#include "expr.h"

START_TEST(test_expr_int)
{
    expr_t *expr_int = (expr_t*)expr_int_make(10);

    ck_assert(expr_eval(expr_int) == 10);

    expr_destroy(expr_int);
}
END_TEST

START_TEST(test_expr_current)
{
    expr_t *expr_current = expr_current_make();

    expr_eval_current(&expr_current, 145);

    ck_assert(expr_eval(expr_current) == 145);

    expr_destroy(expr_current);
}
END_TEST

START_TEST(test_expr_label)
{
    hashmap_t *label_map = hashmap_make();
    hashmap_insert(label_map, "test_label", 0x420);

    expr_t *expr_label = (expr_t*)expr_label_make("test_label");

    expr_eval_labels(&expr_label, label_map);

    ck_assert(expr_eval(expr_label) == 0x420);

    expr_destroy(expr_label);
    hashmap_destroy(label_map);
}
END_TEST

#define TEST_BINOP(op, lhs, rhs, result)        \
    expr_t *expr = (expr_t*)expr_binop_make(op, \
            (expr_t*)expr_int_make(lhs),        \
            (expr_t*)expr_int_make(rhs));       \
    ck_assert_int_eq(expr_eval(expr), result);  \
    expr_destroy(expr);

START_TEST(test_expr_add)
{
    TEST_BINOP(EXPR_ADD, 15, 66, (15 + 66));
}
END_TEST

START_TEST(test_expr_sub)
{
    TEST_BINOP(EXPR_SUB, 66, 15, (66 - 15));
}
END_TEST

START_TEST(test_expr_mul)
{
    TEST_BINOP(EXPR_MUL, 6, 7, 42);
}
END_TEST

START_TEST(test_expr_div)
{
    TEST_BINOP(EXPR_DIV, 45, 6, (45 / 6));
}
END_TEST

START_TEST(test_expr_mod)
{
    TEST_BINOP(EXPR_MOD, 543, 26, (543 % 26));
}
END_TEST

Suite * expr_suite(void)
{
    Suite *suite;
    TCase *tc_core;

    suite = suite_create("Expressions");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_expr_int);
    tcase_add_test(tc_core, test_expr_current);
    tcase_add_test(tc_core, test_expr_label);
    tcase_add_test(tc_core, test_expr_add);
    tcase_add_test(tc_core, test_expr_sub);
    tcase_add_test(tc_core, test_expr_mul);
    tcase_add_test(tc_core, test_expr_div);
    tcase_add_test(tc_core, test_expr_mod);
    suite_add_tcase(suite, tc_core);

    return suite;
}
