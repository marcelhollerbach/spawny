#ifndef DESKTOP_COMMON_H
#define DESKTOP_COMMON_H

#include "templatereg.h"
#include "ini.h"

void parse_dir(const char *directory_path, Template_Fire_Up fire_up);

/**
 * 0 on fail
 * 1 on success
 */
int parse_ini_verbose(const char* filename, ini_handler handler, void* user);

#endif