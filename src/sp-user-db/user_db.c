#include "user_db.h"

#include "ini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#define INI ".ini"
#define S_INI (sizeof(INI) - 1)
#define SPAWNY_USER_DB "/var/lib/spawny/"

#define SAFETY_CHECK(val) do { if (!val) { printf(# val " is NULL\n"); return; }}while(0)
#define SAFETY_CHECK_RET(val, ret) do { if (!val) { printf(# val " is NULL\n"); return ret; }}while(0)

#define ERRNO_PRINTF(val, ...) \
    printf(val"\n -> Reason: %s\n", ##__VA_ARGS__ ,strerror(errno));

typedef struct {
    char *value;
    char *name;
} Pair;

typedef struct _User{
    char *name;
    Pair *additional; /* NULL terminated array of additional infos */
    unsigned n_additional;
} User;

typedef struct {
    uid_t started_uid;
    User *user;
    unsigned int n_user;
    int ref;
} Context;

static Context *ctx = NULL;

#define STARTED_AS_ROOT (ctx->started_uid == 0)

static Pair*
_user_pair_add(User *user) {
    user->n_additional ++;
    user->additional = realloc(user->additional, sizeof(Pair)* user->n_additional);

    memset(&user->additional[user->n_additional - 1], 0, sizeof(Pair));

    return &user->additional[user->n_additional - 1];
}

static void
_user_pair_remove(User *user, const char *name) {

    bool swap = false;

    for(int i = 0; i < user->n_additional; i++) {
        if (!swap) {
            if (!strcmp(user->additional[i].name, name)) swap = true;
        } else {
            memcpy(&user->additional[i] , &user->additional[i + 1], sizeof(Pair));
        }
    }

    user->n_additional --;

    user->additional = realloc(user->additional, sizeof(Pair)* user->n_additional);
}

static User*
_user_add(void) {
    User usr = { NULL };
    User *new_user;

    ctx->n_user ++;
    ctx->user = realloc(ctx->user, sizeof(User)* ctx->n_user);

    new_user = &ctx->user[ctx->n_user - 1];

    memcpy(new_user, &usr, sizeof(User));

    return new_user;
}

static User*
_user_find(const char *name)
{
    for (int i = 0; i < ctx->n_user; i++) {
        User *usr = &ctx->user[i];
        if (!strcmp(usr->name, name)) {
            return usr;
        }
    }
    return NULL;
}

static Pair*
_user_field_get(User *user, const char *field)
{
    for(int i = 0; i < user->n_additional; i++) {
        if (!strcmp(user->additional[i].name, field))
          return &user->additional[i];
    }
    return NULL;
}
static void
_user_free_additional(User *user) {
    for(int i = 0; i < user->n_additional; i++) {
        free(user->additional[i].value);
        free(user->additional[i].name);
    }

    free(user->additional);
    user->additional = NULL;
    user->n_additional = 0;
}

static void
_user_list_free(void) {
    for(int i = 0; i < ctx->n_user; i++) {
        _user_free_additional(&ctx->user[i]);
    }

    free(ctx->user);
    ctx->user = NULL;
    ctx->n_user = 0;
}
static void
_list_users(char ***usernames, unsigned int *numb) {
    char line[PATH_MAX];
    FILE *fptr;
    char *username;
    char **names = NULL;

    fptr = fopen("/etc/passwd","r");

    if (!fptr) {
        ERRNO_PRINTF("Failed to read /etc/passwd");
        return;
    }

    *numb = 0;
    *usernames = NULL;

    if (fptr == NULL) {
         perror("Failed to read passwd");
         return;
    }

    while (fgets(line, sizeof(line), fptr)) {
        username = strtok(line, ":");

        (*numb) ++;

        names = realloc(names, (*numb) * sizeof(char*));
        names[(*numb) - 1] = strdup(username);
    }
    *usernames = names;
    fclose(fptr);
}

static bool
_put_user(User *user)
{
    char buf[4048];
    char file[PATH_MAX];
    FILE *f;
    int fd;
    size_t len;
    struct passwd *pw;
    const mode_t mode = (S_IRUSR | S_IWUSR | S_IROTH);

    pw = getpwnam(user->name);

    /* generate filename */
    snprintf(file, sizeof(file), SPAWNY_USER_DB"%s.ini", user->name);

    /* open file */
    f = fopen(file, "w+");
    if (!f) {
        ERRNO_PRINTF("Failed to open file %s", file);
        return false;
    }

    if (!!chown(file, pw->pw_uid , pw->pw_gid)) {
        ERRNO_PRINTF("Failed to chown file %s", file);
        return false;
    }

    if (!!chmod(file, mode)) {
        ERRNO_PRINTF("Failed to chmod file %s", file);
        return false;
    }

    fd = fileno(f);

    flock(fd, LOCK_EX);

    /* delete everything */
    len = snprintf(buf, sizeof(buf), "[identify]\nname=%s\n[info]\n", user->name);
    fwrite(buf, len, sizeof(char), f);

    /* write infos */
    for(int i = 0; i < user->n_additional; i++) {
        len = snprintf(buf, sizeof(buf), "%s=%s\n", user->additional[i].name, user->additional[i].value);
        fwrite(buf, len, sizeof(char), f);
    }

    fflush(f);

    flock(fd, LOCK_UN);

    fclose(f);

    return true;
}

static int
_load_ini_parse(void* data, const char* section, const char* name, const char* value)
{
    User *user = data;

    if (!strcmp(section, "identify")) {
        if (!strcmp(name, "name")) {
            user->name = strdup(value);
        } else {
            return 0;
        }
    } else if (!strcmp(section, "info")) {
        Pair *pair = _user_pair_add(user);

        pair->value = strdup(value);
        pair->name = strdup(name);
    } else {
        return 0;
    }
    return 1;
}


static void
_load_file(const char *filename)
{
    char buf[PATH_MAX];
    FILE* file;
    User *listuser;
    User user = { NULL };
    struct passwd *pw;


    snprintf(buf, sizeof(buf), SPAWNY_USER_DB"%s", filename);

    file = fopen(buf, "r");

    if (!file) {
        ERRNO_PRINTF("Opening file %s failed.", filename)
        return;
    }

    ini_parse_file(file, _load_ini_parse, &user);

    if (!user.name) {
        printf("File %s does not specify name in section identify\n", filename);
        return;
    }

    pw = getpwnam(user.name);
    if (!pw) {
        printf("Failed to fetch struct passwd from %s with name %s\n", filename, user.name);
        return;
    }

    if(_user_find(user.name)) {
        printf("Duplucated entry for %s in file %s\n", user.name, filename);
        return;
    }

    listuser = _user_add();
    memcpy(listuser, &user, sizeof(User));
}

static void
_load_from_fs(void) {
    DIR* od;
    struct dirent *dir;

    od = opendir(SPAWNY_USER_DB);

    if (!od) {
        ERRNO_PRINTF("Opening database %s failed.", SPAWNY_USER_DB);
        return;
    }

    while ((dir = readdir(od))) {
        char *filename = dir->d_name;
        int len = strlen(filename);
        if (len > S_INI && !strncmp(filename + len - S_INI, INI, S_INI)) {
            _load_file(filename);
        }
    }
}

bool
user_db_sync(void) {
    char **users;
    unsigned int users_n;
    int damage = 0;

    if (!STARTED_AS_ROOT) {
        printf("Syncing with passwd is only allowed as root.");
        return false;
    }

    _list_users(&users, &users_n);

    for (int i = 0; i < users_n; i++) {
        char *username = users[i];
        User *user = NULL;
        User tmp_usr = { NULL };

        user = _user_find(username);

        if (user) continue;

        tmp_usr.name = username;

        damage = 1;

        _put_user(&tmp_usr);
    }

    if (!damage) return true;

    /* free everything */
    _user_list_free();

    /* load new from fs */
    _load_from_fs();

    return true;
}

int
user_db_users_iterate(char ***users) {
    *users = calloc(ctx->n_user, sizeof(char*));

    for(int i = 0; i < ctx->n_user; i++) {
        (*users)[i] = strdup(ctx->user[i].name);
    }
    return ctx->n_user;
}

int
user_db_init(void) {

    if (!ctx) {
        ctx = calloc(1, sizeof(Context));
        ctx->started_uid = getuid();

        _load_from_fs();

        if (STARTED_AS_ROOT) {
            user_db_sync();
        } else if (!ctx->n_user) {
            printf("No user found! Run as root to initializie the db.\n");
        }
        ctx->ref = 1;
    } else {
        ctx->ref ++;
    }

    return 1;
}

void
user_db_shutdown(void) {
    ctx->ref --;

    if (ctx->ref == 0) {
        _user_list_free();
        free(ctx);
        ctx = NULL;
    }
}

bool
user_db_user_exists(const char *name)
{
    SAFETY_CHECK_RET(name, false);

    return !!_user_find(name);
}

const char*
user_db_field_get(const char *name, const char *field)
{
    Pair *pair;
    User *user;

    SAFETY_CHECK_RET(name, NULL);
    SAFETY_CHECK_RET(field, NULL);

    user = _user_find(name);
    pair = _user_field_get(user, field);

    if (!pair) return NULL;

    return pair->value;
}

bool
user_db_field_set(const char *name, const char *field, const char *value)
{
    Pair *pair;
    User *user;
    struct passwd *pw;

    SAFETY_CHECK_RET(name, false);
    SAFETY_CHECK_RET(field, false);
    SAFETY_CHECK_RET(value, false);

    pw = getpwnam(name);

    if (pw->pw_uid != getuid()) {
        printf("Editing is only allowed for the getuid() user\n");
        return false;
    }

    user = _user_find(name);
    pair = _user_field_get(user, field);

    if (!pair) {
        pair = _user_pair_add(user);
        pair->name = strdup(field);
    } else {
        free(pair->value);
    }

    pair->value = strdup(value);

    if (!_put_user(user)) {
        return false;
    }
    _user_list_free();
    _load_from_fs();

    return true;
}

bool
user_db_field_del(const char *name, const char *field)
{
    User *user;

    SAFETY_CHECK_RET(name, false);
    SAFETY_CHECK_RET(field, false);

    user = _user_find(name);

    _user_pair_remove(user, field);

    if (!_put_user(user)) {
        return false;
    }
    _user_list_free();
    _load_from_fs();

    return true;
}
