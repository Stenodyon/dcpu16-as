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

#include <stdint.h>

#include "hashmap_tests.h"
#include "hashmap.h"

#include "hashmap_tests_values.h"

START_TEST(test_hashmap_make)
{
    hashmap_t *hashmap = hashmap_make();

    ck_assert(hashmap != NULL);

    hashmap_destroy(hashmap);
}
END_TEST

START_TEST(test_hashmap_insert)
{
    hashmap_t *hashmap = hashmap_make();

    hashmap_insert(hashmap, "test_str", 0x14);
    ck_assert(hashmap_lookup(hashmap, "test_str") == 0x14);

    hashmap_destroy(hashmap);
}
END_TEST

START_TEST(test_hashmap_overwrite)
{
    hashmap_t *hashmap = hashmap_make();

    hashmap_insert(hashmap, "some_string", 0x45);
    hashmap_insert(hashmap, "some_string", 0x31);

    ck_assert(hashmap_lookup(hashmap, "some_string") == 0x31);

    hashmap_destroy(hashmap);
}
END_TEST

START_TEST(test_hashmap_expand)
{
    hashmap_t *hashmap = hashmap_make();

    // Insert
    for (int i = 0; i < 256; i++)
    {
        const char * label = names[i];
        const uint16_t value = values[i];
        hashmap_insert(hashmap, label, value);
    }

    // Test
    for (int i = 0; i < 256; i++)
    {
        const char * label = names[i];
        const uint16_t value = values[i];
        const uint16_t returned = hashmap_lookup(hashmap, label);
        ck_assert(returned == value);
    }

    hashmap_destroy(hashmap);
}
END_TEST

START_TEST(test_hashmap_doesnt_contain)
{
    hashmap_t *hashmap = hashmap_make();

    ck_assert(hashmap_lookup(hashmap, "hello world") == -1);

    hashmap_destroy(hashmap);
}
END_TEST

Suite * hashmap_suite(void)
{
    Suite *suite;
    TCase *tc_core;

    suite = suite_create("Hashmap");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_hashmap_make);
    tcase_add_test(tc_core, test_hashmap_insert);
    tcase_add_test(tc_core, test_hashmap_expand);
    tcase_add_test(tc_core, test_hashmap_overwrite);
    tcase_add_test(tc_core, test_hashmap_doesnt_contain);
    suite_add_tcase(suite, tc_core);

    return suite;
}
