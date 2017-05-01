#include "test.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <check.h>

static void
_interrupt_handler(int sig)
{
    daemon_stop();
    teardown();
    exit(-1);
}

int main(int argc, char const *argv[])
{
  Suite *s;
  SRunner *sr;
  /**
   * Test suite MUST be run as root since we are calling chroot and we use PRELOAD
   */
  if (getuid() != 0)
    {
       printf("Error, this test suite must be run as root!\n");
       return EXIT_FAILURE;
    }

  signal(SIGINT, _interrupt_handler);
  if (!prepare()) return EXIT_FAILURE;

  s = suite_create("Spawny-Integration-Test");
  suite_add_tcase(s, tcase_protocol());
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  srunner_ntests_failed(sr);
  srunner_free(sr);

  sleep(1);
  if (!teardown()) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
