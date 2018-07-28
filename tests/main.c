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
#include <stdio.h>

#include "hashmap_tests.h"
#include "expr_tests.h"

int main(void)
{
    Suite * suites[] = {
        hashmap_suite(),
        expr_suite()
    };

    int suite_count = sizeof(suites) / sizeof(Suite*);

    SRunner *suite_runner = srunner_create(suites[0]);
    for (int i = 1; i < suite_count; i++)
    {
        Suite * suite = suites[i];
        srunner_add_suite(suite_runner, suite);
    }

    srunner_run_all(suite_runner, CK_NORMAL);
    int number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);

    return (number_failed == 0) ? 0 : -1;
}
