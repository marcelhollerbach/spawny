#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    struct {
        const char *start_user;
        const char *cmd;
    } greeter;
} Config;

extern Config *config;

int config_init(void);
int config_shutdown(void);

#endif