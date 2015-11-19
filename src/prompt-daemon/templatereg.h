#ifndef TEMPLATEREG_H
#define TEMPLATEREG_H

typedef void (*Template_Fire_Up)(void);

const char* template_register(char *name, char *icon, Template_Fire_Up fire_up);
int template_unregister(const char *id);
int template_details_get(const char *id, const char **name, const char **icon);
int template_run(const char *id);
int template_get(char ***template_ids, unsigned int *num);

#endif