#include "test.h"
#include "../sp-daemon/templatereg.h"

int test = 0;

static void
_fire_up_daed(void *data UNUSED) {
    test = 1;
}

static void
_fire_up2(void *data UNUSED) {

}

START_TEST(test_templatereg_details)
{
    int ret;
    const char *id;
    const char *tmp_name, *tmp_icon, *tmp_type;
    char *name = "test";
    char *icon = "icon";
    char *type = "wl";

    id = template_register("1", "2", "test", NULL, NULL);
    id = template_register("no", "3", "test", NULL, NULL);
    id = template_register(name, icon, type, _fire_up_daed, NULL);

    ck_assert_ptr_ne(id, NULL);

    ret = template_details_get(id, &tmp_name, &tmp_icon, &tmp_type);
    ck_assert(ret);

    ck_assert_str_eq(tmp_name, name);
    ck_assert_str_eq(tmp_icon, icon);
    ck_assert_str_eq(tmp_type, type);
}
END_TEST


START_TEST(test_templatereg_run)
{
    const char *id;
    char *name = "test";
    char *icon = "icon";

    id = template_register(name, icon, "t", _fire_up_daed, NULL);

    ck_assert_ptr_ne(id, NULL);

    test = 0;

    template_run(id);

    ck_assert_int_eq(test, 1);
}
END_TEST


START_TEST(test_templatereg_unreg)
{
    int ret;
    const char *id1, *id2, *id3;
    char *name = "test";
    char *icon = "icon";

    id1 = template_register("1", "2", "t", _fire_up2, NULL);
    id2 = template_register("no", "3", "t", _fire_up2, NULL);
    id3 = template_register(name, icon, "t", _fire_up_daed, NULL);


    ck_assert_ptr_ne(id1, NULL);
    ck_assert_ptr_ne(id2, NULL);
    ck_assert_ptr_ne(id3, NULL);

    id2 = strdup(id2);

    ret = template_unregister(id2);
    ck_assert_int_ne(ret, 0);

    //check for crashys
    ret = template_run(id2);
    ck_assert_int_eq(ret, 0);
}
END_TEST

void template_suite(Suite *s)
{
    TCase *tc_core;

    /* Core test case */
    tc_core = tcase_create("template registry");

    tcase_add_test(tc_core, test_templatereg_run);
    tcase_add_test(tc_core, test_templatereg_details);
    tcase_add_test(tc_core, test_templatereg_unreg);
    suite_add_tcase(s, tc_core);
}

