#include "Sp_Client.h"
#include <stdlib.h>

int
main(int argc, char **argv)
{
    sp_client_init(SP_CLIENT_LOGIN_PURPOSE_START_GREETER);
    return EXIT_SUCCESS;
}