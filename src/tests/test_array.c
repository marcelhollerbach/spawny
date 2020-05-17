#include "../sp-daemon/array.h"
#include "test.h"

typedef struct
{
   unsigned int x;
} Tester;

ARRAY_API(Tester)

START_TEST(array_test)
{
   Array *arr;
   Tester *t, *t2;

   arr = array_Tester_new();
   t = array_Tester_add(arr);
   t->x = 20;
   t2 = array_Tester_get(arr, 0);

   ck_assert_ptr_eq(t, t2);
   ck_assert_int_eq(t->x, 20);

   array_Tester_del(arr, 0);

   ck_assert_int_eq(array_len_get(arr), 0);

   array_free(arr);
}
END_TEST

void
array_suite(Suite *s)
{
   TCase *tc_core;

   /* Core test case */
   tc_core = tcase_create("core");
   tcase_add_test(tc_core, array_test);
   suite_add_tcase(s, tc_core);
}
