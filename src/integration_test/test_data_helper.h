#ifndef TEST_DATA_HELPER
#define TEST_DATA_HELPER

#include <Sp_Client.h>
#include <check.h>

#define EQ(type, name) ck_assert_ ##type ##_eq(a->name, b->name)

void
user_equal(User *a, User *b)
{
   EQ(str, icon);
   EQ(str, name);
   //EQ(int, id);
   EQ(int, prefered_session);
}

void
template_equal(Template *a, Template *b)
{
   //EQ(int, id);
   EQ(str, icon);
   EQ(str, name);
}

void
session_equal(Session *a, Session *b)
{
   //EQ(int, id);
   EQ(str, icon);
   EQ(str, user);
   EQ(str, name);
}

#endif
