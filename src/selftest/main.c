#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <security/pam_appl.h>

typedef struct {
   const char *user;
   const char *password;
   bool autologin;
} Pam_Config;

static void
check_user(const char *user)
{
   struct passwd *user_entry;

   user_entry = getpwnam(user);

   if (!user_entry)
     {
        printf("FATAL: Failed to find user %s\n", user);
        abort();
     }
   else
     {
        printf("Users are looking good!\n");
     }
}

static const char*
_prompt(const char *label, bool echo)
{
   printf("%s", label);
   struct termios old, new;
   char buffer[LOGIN_NAME_MAX];

   if (!echo) {
        tcgetattr(fileno(stdin), &old);
        new = old;
        new.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), TCSAFLUSH, &new);
   }
   scanf("%s", buffer);
   if (!echo) {
        tcsetattr(fileno(stdin), TCSAFLUSH, &old);
   }

   return strdup(buffer);
}

static int
_conversation(int num_msg, const struct pam_message **msg,
         struct pam_response **rp, void *appdata_ptr) {
   int i = 0;

   Pam_Config *c = appdata_ptr;

   *rp = (struct pam_response *) calloc(num_msg, sizeof(struct pam_response));
   for (i = 0; i < num_msg; i++)
     {
        rp[i]->resp_retcode = 0;
        switch(msg[i]->msg_style)
          {
             case PAM_PROMPT_ECHO_ON:
                if (!c->user) c->user = _prompt(msg[i]->msg, true);
                rp[i]->resp = strdup(c->user);
                break;
             case PAM_PROMPT_ECHO_OFF:
                if (c->autologin) {
                  printf("Password requested in autologin-mode!\n");
                  abort();
                }
                if (!c->password) c->password = _prompt(msg[i]->msg, false);
                rp[i]->resp = strdup(c->password);
                break;
             case PAM_TEXT_INFO:
             case PAM_ERROR_MSG:
                printf("%s\n", msg[i]->msg);
                break;
             default:

                break;
          }
     }
   return PAM_SUCCESS;
}

static void
_handle_error(int ret) {

    if (ret == PAM_SUCCESS) return;

    #define RET_VAL_REG(v, val) \
    case v: \
        printf("Pam error "val" happend\n"); \
        break;

    switch(ret){
        RET_VAL_REG(PAM_BUF_ERR, "PAM_BUF_ERR");
        RET_VAL_REG(PAM_CONV_ERR, "PAM_CONF_ERR");
        RET_VAL_REG(PAM_ABORT, "PAM_ABORT");
        RET_VAL_REG(PAM_SYSTEM_ERR, "PAM_SYSTEM_ERR");
        RET_VAL_REG(PAM_AUTH_ERR, "PAM_AUTH_ERR");
        RET_VAL_REG(PAM_CRED_INSUFFICIENT, "PAM_CRED_INSUFFICIENT");
//FIXME WTF THIS IS FROM THE DOC         RET_VAL_REG(PAM_AUTHINFO_UNVAIL, "PAM_AUTHINFO_UNVAIL");
        RET_VAL_REG(PAM_MAXTRIES, "PAM_MAX_TRIES");
        RET_VAL_REG(PAM_USER_UNKNOWN, "PAM_USER_UNKNOWN");
        default:
            printf("uncaught state\n");
        break;
    }
    #undef RET_VAL_REG

    printf("PAM TEST FAILED!\nCheck your pam setup!\n");
    abort();
}

static void
check_pam_service(const char *pam_service, Pam_Config raw_config)
{
   int ret;
   struct pam_conv conv;
   pam_handle_t *handle;
   Pam_Config config = raw_config;

   printf("Checking service '%s'\n", pam_service);

   conv.appdata_ptr = &config;
   conv.conv = _conversation;

   ret = pam_start(pam_service, NULL, &conv, &handle);
   _handle_error(ret);
   ret = pam_authenticate(handle, 0);
   _handle_error(ret);
   ret = pam_setcred(handle, PAM_ESTABLISH_CRED);
    _handle_error(ret);
   ret = pam_open_session(handle, 0);
   _handle_error(ret);
   ret = pam_end(handle, ret);

   printf("Sucessfully checked pam service '%s'\n", pam_service);
}

static void
print_help(void)
{
   printf("run with <app> username password, or no argument at all!\n");
}

int main(int argc, char const *argv[])
{
  Pam_Config config = {NULL, NULL, 0};
  printf("Welcome to selftest!\n");

  if (argc != 3 && argc != 1)
    {
       print_help();
       return EXIT_FAILURE;
    }

  if (argc == 3) {
    config.user = argv[1];
    config.password = argv[2];
  }

  check_user(SP_USER);

  config.autologin = false;
  check_pam_service(SP_PAM_SERVICE, config);

  config.autologin = true;
  check_pam_service(SP_PAM_SERVICE_GREETER, config);

  return 0;
}
