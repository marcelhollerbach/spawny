#include "Sp_Client.h"
#include <stdlib.h>

int
main(int argc, char **argv)
{
    if (!sp_client_greeter_start(argc, argv))
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
