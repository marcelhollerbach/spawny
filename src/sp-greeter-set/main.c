#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static void
print_help(void)
{
   printf("Usage: sp-greeter-set <path-to-bin>\n");
   printf("<path-to-bin> has to exist and must be readable\n");
}

int main(int argc, char const *argv[])
{
  if (argc != 2)
    {
        print_help();
        return EXIT_FAILURE;
    }

  if (access(argv[1], R_OK) != 0)
    {
        print_help();
        return EXIT_FAILURE;
    }

  if (access(GREETER_BINARY_PATH, R_OK) != 0)
    {
       unlink(GREETER_BINARY_PATH);
    }

  if (!link(argv[1], GREETER_BINARY_PATH))
    {
       perror("Failed to link argument to "GREETER_BINARY_PATH);
       return EXIT_FAILURE;
    }
  printf("Setted %s as default.\n", argv[1]);
  return EXIT_SUCCESS;
}
