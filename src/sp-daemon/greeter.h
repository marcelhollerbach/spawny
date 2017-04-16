#ifndef GREETER_H
#define GREETER_H

/*activates a greeter */
void greeter_init(void);
void greeter_activate(const char *seat);
void greeter_lockout(const char *seat);
void greeter_shutdown(void);
bool greeter_exists_sid(pid_t sid);

#endif
