#include <check.h>
#include <stdio.h>

#include "hashmap_tests.h"

int main(void)
{
    Suite * suite = hashmap_suite();
    SRunner * suite_runner = srunner_create(suite);

    srunner_run_all(suite_runner, CK_NORMAL);
    int number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);

    return (number_failed == 0) ? 0 : -1;
}
