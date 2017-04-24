#include "main.h"
#include <string.h>
#include "ini.h"
#include <errno.h>

Config *config;

static int
_config_parse(void* user, const char* section,
              const char* name, const char* value) {
    if (!strcmp(section, "greeter")) {
        if (!strcmp(name, "user"))
          config->greeter.start_user = strdup(value);
        else if (!strcmp(name, "cmd"))
          config->greeter.cmd = strdup(value);
        else
          return 0;
    } else {
      return 0;
    }

    return 1;
}

int
config_check_correctness(void) {
   int ret = 1;
   //check if command exists
   if (config->greeter.cmd) {
        if (access(config->greeter.cmd, R_OK) != 0) {
          ERR("Failed to access %s. Error %s\n", config->greeter.cmd, strerror(errno));
          ret = 0;
        }
   }
   errno = 0;
   if (!getpwnam(config->greeter.start_user)) {
        ERR("User %s is not known on the system. Error: %s", config->greeter.start_user, strerror(errno));
        ret = 0;
   }
   return ret;
}

int
config_init(void) {
    config = calloc(1, sizeof(Config));

    config->greeter.start_user = "spawny";
    config->greeter.cmd = NULL;

    mkpath(PACKAGE_CONFIG, S_IRUSR);

    parse_ini_verbose(PACKAGE_CONFIG, _config_parse, NULL);

    return 1;
}

int
config_shutdown(void) {
    free(config);
    config = NULL;
    return 1;
}
