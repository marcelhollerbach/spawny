#include "main.h"

#include <string.h>
#include <unistd.h>

Global _G;

static void
init_check(void) {
    const char *session;

    if (getuid() != 0) {
        ERR("You can only run this as root.");
        exit(-1);
    }

    session = current_session_get();

    if (session != NULL) {
        ERR("You must start this out of a session. This is in session %s", session);
        exit(-1);
    }
}

void
print_help(void)
{
    printf("Supported options\n");
    printf("--debug / -d Start the daemon in debug mode\n");
    printf("--no-greeter-restart Do not start the greeter once a session has shutted down\n", );
}

void
_parse_args(int argc, char *argv[])
{
   _G.argc = argc;
   _G.argv = argv;
   _G.config.debug = false;
   _G.config.no_greeter_restart = false;

   for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")) {
            _G.config.debug = true;
        } else if (!strcmp(argv[i], "--no-greeter-restart")) {
            _G.config.no_greeter_restart = true;
        } else {
            print_help();
            exit(-1);
        }
   }
}

int
main(int argc, char **argv)
{
    _parse_args(argc, argv);
    log_init();
    init_check();
    config_init();
    manager_init();
    greeter_init();
    spawnservice_init();
    spawnregistery_init();

    tty_template_init();
    x11_template_init();
    wl_template_init();

    if (!server_init()) {
        return -1;
    }

    manager_run();

    server_shutdown();
    greeter_shutdown();
    template_shutdown();
    config_shutdown();
}
