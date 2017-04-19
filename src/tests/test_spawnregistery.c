#include "test.h"
#include "../sp-daemon/spawnregistery.h"
#include <unistd.h>
#include <stdbool.h>

static pid_t
child_create(void)
{
   pid_t child = fork();

   if (child == 0) {
      abort();
   } else {
      //make sure the child is really really created
      sleep(2);
   }
   return child;
}

static void
_handler(void *data, int sig, pid_t t)
{
    bool *b = data;
    ck_assert_int_eq(*b, false);
    ck_assert(WIFSIGNALED(SIGABRT));
    *b = true;
}

START_TEST(spawnregistery_callback_test)
{
   bool b = false;
   pid_t child_pid;

   spawnregistery_init();

   child_pid = child_create();
   child_create();

   spawnregistery_listen(child_pid, _handler, &b);

   spawnregistery_eval();
   ck_assert_int_eq(b, true);

}
END_TEST

void spawnregistery_suite(Suite *s)
{
    TCase *tc_core;

    /* Core test case */
    tc_core = tcase_create("Spawnregistery");
    tcase_set_timeout(tc_core, 6);
    tcase_add_test(tc_core, spawnregistery_callback_test);
    suite_add_tcase(s, tc_core);
}

