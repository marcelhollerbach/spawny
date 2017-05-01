#ifndef TEST_H
#define TEST_H

#include "binary_locations.h"
#include "config.h"

#include <stdbool.h>
#include <check.h>

/**
 * Setup the chroot environment
 */
bool prepare(void);
bool teardown(void);

void daemon_start(void);
void daemon_stop(void);

TCase* tcase_greeter_start(void);
TCase* tcase_protocol(void);


#endif
