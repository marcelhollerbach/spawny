#include "test.h"

typedef void (*Suite_Creator)(Suite *s);

Suite_Creator spawny_suites[] = {
  array_suite,
  manager_suite,
  template_suite,
  spawnregistery_suite,
  NULL
};


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = suite_create("Spawny");

    for(int i = 0; spawny_suites[i]; i++)
      {
         spawny_suites[i](s);
      }

    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
