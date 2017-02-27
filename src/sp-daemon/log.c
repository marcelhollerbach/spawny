#include "main.h"
#include <libgen.h>
#include <stdarg.h>

static int min_log_level;

void
log_init(void)
{
   const char *raw;

   min_log_level = 0;

   raw = getenv("SPAWNY_LOG_LEVEL");

   if (raw)
     min_log_level = atoi(raw);
}

void
log_to_type(Log_Type type, char *file, int line_number, const char *msg, ...)
{
   char *file_name;
   va_list args;

   if (type < min_log_level)
     return;

   //print welcomer
   if (type == LOG_TYPE_ERR)
     printf("\x1B[31mERR");
   else if (type == LOG_TYPE_INF)
     printf("\x1B[36mINF");

   file_name = basename(file);
   //print filename
   printf("\x1B[37m<%s:%d>", file_name, line_number);

   printf("\x1B[0m");
   va_start(args, msg);
   vprintf(msg, args);
   va_end(args);

   printf("\n");
}
