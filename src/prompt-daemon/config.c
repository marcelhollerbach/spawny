#include "main.h"

Config *config;

int
config_init(void) {
    config = calloc(1, sizeof(Config));

    config->greeter.start_user = "spawny";
    config->greeter.cmd = "/usr/local/share/data/spawny-clc";
    //config->greeter.cmd = "/usr/bin/bash";
    return 1;
}

int
config_shutdown(void) {
    return 1;
}
