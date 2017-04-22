#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "log.h"
#include "sp_protocol.h"
#include "config.h"

#include "app_config.h"
#include "spawnservice.h"
#include "desktop_common.h"
#include "manager.h"
#include "sessionmgt.h"
#include "main.h"
#include "server.h"
#include "templatereg.h"
#include "templates.h"
#include "spawnregistery.h"
#include "greeter.h"
#include "user_db.h"
#include "array.h"

typedef struct {
  struct {
     bool debug;
     bool no_greeter_restart;
  } config;

  int argc;
  char **argv;
} Global;

extern Global _G;

static inline int
INT_LENGTH(int value) {
  return (value == 0 ? 1 : ((int)(log10(abs(value))+1) + (value < 0 ? 1 : 0)));
}

#endif
