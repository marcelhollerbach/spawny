#ifndef TEST_DATA
#define TEST_DATA

#include <Sp_Client.h>
#include <check.h>

/*
root:x:0:0:root:/root:/bin/bash
spawny:x:888:888:Spawny Login user:/var/lib/spawny:/usr/bin/nologin
user1:x:1000:100::/home/marcel:/bin/bash
user2:x:1000:100::/home/marcel:/bin/bash
user3:x:1000:100::/home/marcel:/bin/bash
*/

/** mirrored from the passwd file **/
User ref_users[] = {
   {2,"","user1",0},
   {3,"","user3",0},
   {1,"","spawny",0},
   {3,"","user2",0},
   {0,"","root",0},
};

Template ref_templates[] = {
   {0,"utilities-terminal", "tty"},
};

Session ref_sessions[] = {
   {0, "", "root", "wayland - Ultimate Testing environment"},
   {1, "", "root", "wayland - Ultimate Testing environment"},
};

#endif
