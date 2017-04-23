#include <unistd.h>

#include "test.h"
#include "../sp-daemon/utils.h"
#include <errno.h>
#include <stdio.h>

START_TEST(create_recusrive_dir)
{
    const char *testdirprepre = "/tmp/spawny-recursive-test/";
    const char *testdirpre = "/tmp/spawny-recursive-test/test/";
    const char *testdir = "/tmp/spawny-recursive-test/test/test";

    rmdir(testdir);
    rmdir(testdirpre);
    rmdir(testdirprepre);

    ck_assert_int_eq(mkpath(testdir, S_IRUSR | S_IWUSR | S_IXUSR), 0);
}
END_TEST

void util_suite(Suite *s)
{
    TCase *tc_core;

    /* Core test case */
    tc_core = tcase_create("core");
    tcase_add_test(tc_core, create_recusrive_dir);
    suite_add_tcase(s, tc_core);
}

