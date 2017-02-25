#include "main.h"
#include <string.h>
#include "ini.h"

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
config_init(void) {
    config = calloc(1, sizeof(Config));

    config->greeter.start_user = "spawny";
    config->greeter.cmd = NULL;

    parse_ini_verbose(PACKAGE_CONFIG, _config_parse, NULL);

    return 1;
}

int
config_shutdown(void) {
    return 1;
}
