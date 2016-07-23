#include "../client/Spawny_Client.h"

int
main(int argc, char **argv)
{
    sp_client_init(SP_CLIENT_LOGIN_PURPOSE_START_GREETER);

    sp_client_start_greeter();

    sp_client_shutdown();

    return 1;
}