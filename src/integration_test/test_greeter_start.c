#include <check.h>

START_TEST(greeter_start)
{

}
END_TEST

TCase*
tcase_greeter_start(void) {
   TCase *c;

   c = tcase_create("greeter_start");

   tcase_add_test(c, greeter_start);

   return c;
}
