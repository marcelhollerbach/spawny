#include "log.h"
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static Log_Type min_log_level;

void
log_init(void)
{
   const char *raw;
   int _level = 0;

   raw = getenv("SPAWNY_LOG_LEVEL");

   if (!raw)
      return;

   _level = atoi(raw);
   if (_level == 0)
      min_log_level = LOG_TYPE_INF;
   else if (_level >= 1)
      min_log_level = LOG_TYPE_ERR;
}

void
log_to_type(Log_Type type, char *file, int line_number, const char *msg, ...)
{
   char *file_name;
   va_list args;

   if (type < min_log_level)
      return;

   // print welcomer
   if (type == LOG_TYPE_ERR)
      printf("\x1B[31mERR");
   else if (type == LOG_TYPE_INF)
      printf("\x1B[36mINF");

   file_name = basename(file);
   // print filename
   printf("\x1B[37m<%s:%d>", file_name, line_number);

   printf("\x1B[0m");
   va_start(args, msg);
   vprintf(msg, args);
   va_end(args);

   fflush(stdout);

   printf("\n");
}
