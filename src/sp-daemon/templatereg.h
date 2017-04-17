#ifndef TEMPLATEREG_H
#define TEMPLATEREG_H

typedef void (*Template_Fire_Up)(void *data);

void template_shutdown(void);
const char* template_register(const char *name, const char *icon, const char *type, Template_Fire_Up fire_up, void *data);
int template_unregister(const char *id);
int template_details_get(const char *id, const char **name, const char **icon, const char **type);
int template_run(const char *id);
int template_get(char ***template_ids, unsigned int *num);

#endif
