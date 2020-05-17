#ifndef SP_CLIENT_PRIVATE_H
#define SP_CLIENT_PRIVATE_H

#include "config.h"
#include "greeter.pb-c.h"
#include "server.pb-c.h"
#include "sp_protocol.h"

#include <sp-util.h>

#include <sys/un.h>

#define PUBLIC_API __attribute__((visibility("default")))

#endif
