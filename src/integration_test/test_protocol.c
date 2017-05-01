#include "test.h"
#include <Sp_Client.h>
#include <stdlib.h>
#include <stdio.h>

#include "test_data_helper.h"
#include "test_data.h"

static bool data_arrived = false;
static bool feedback_given = false;


static void
_data(void)
{
   data_arrived = true;
}

static void
_login_feedback(int succes, char *msg)
{
   feedback_given = true;
}

static Sp_Client_Interface interface = {
  _data,
  _login_feedback,
};

START_TEST(protocol_test)
{

   Sp_Client_Context *ctx;
   Sp_Client_Read_Result result;

   char *argv[] = {"b", "-d", NULL};

   daemon_start();
   ctx = sp_client_init(2, argv);

   ck_assert_ptr_nonnull(ctx);

   result = sp_client_read(ctx, &interface);
   ck_assert_int_eq(result, SP_CLIENT_READ_RESULT_SUCCESS);
   ck_assert_int_eq(data_arrived, true);
   ck_assert_int_eq(feedback_given, false);

   //now check the data
   {
      Numbered_Array templates, users, sessions;

      sp_client_data_get(ctx, &sessions, &templates, &users);

      for (int i = 0; i < templates.length; ++i) {
        Template *a = &ref_templates[i];
        Template *b = &TEMPLATE_ARRAY(&templates, i);
        template_equal(a, b);
      }

      for (int i = 0; i < users.length; ++i) {
        User *a = &ref_users[i];
        User *b = &USER_ARRAY(&users, i);
        user_equal(a, b);
        //printf("%s\n", b->name);
      }

      for (int i = 0; i < sessions.length; ++i) {
        Session *a = &ref_sessions[i];
        Session *b = &SESSION_ARRAY(&sessions, i);
        session_equal(a, b);
      }
   }

   daemon_stop();
}
END_TEST

TCase*
tcase_protocol(void)
{
   TCase *c;

   c = tcase_create("protocol");

   tcase_add_test(c, protocol_test);

   return c;
}
