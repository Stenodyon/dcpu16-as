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
