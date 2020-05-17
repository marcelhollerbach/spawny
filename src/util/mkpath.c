#include "mkpath.h"
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
mkpath(const char *path, mode_t mode)
{
   char path_str[strlen(path) + 1];
   char *dir;

   snprintf(path_str, sizeof(path_str), "%s", path);
   dir = dirname(path_str);

   if (access(dir, R_OK) < 0) {
      if (mkpath(dir, mode) == -1)
         return -1;
   }

   if (mkdir(path, mode) == -1)
      return -1;

   return 0;
}
