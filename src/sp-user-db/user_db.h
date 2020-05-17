#ifndef USER_DB_H
#define USER_DB_H

#include <pwd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

int
user_db_init(void);
bool
user_db_sync(void);
int
user_db_users_iterate(char ***users);
void
user_db_shutdown(void);

/* accessor functions */
bool
user_db_user_exists(const char *name);
const char *
user_db_field_get(const char *name, const char *field);
bool
user_db_field_set(const char *name, const char *field, const char *value);
bool
user_db_field_del(const char *name, const char *field);

#endif