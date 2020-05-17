#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <sp-util.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config.h"
#include "sp_protocol.h"

#include "array.h"
#include "desktop_common.h"
#include "greeter.h"
#include "main.h"
#include "manager.h"
#include "server.h"
#include "sessionmgt.h"
#include "spawnregistery.h"
#include "spawnservice.h"
#include "templatereg.h"
#include "templates.h"
#include "user_db.h"

typedef struct
{
   struct
   {
      bool debug;
      bool no_greeter_restart;
   } config;

   int argc;
   char **argv;
} Global;

extern Global _G;

static inline int
INT_LENGTH(int value)
{
   return (value == 0 ? 1
                      : ((int)(log10(abs(value)) + 1) + (value < 0 ? 1 : 0)));
}
#define UNUSED __attribute__((unused))

#endif
