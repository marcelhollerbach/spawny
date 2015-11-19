#ifndef MANAGER_H
#define MANAGER_H

typedef void (*Fd_Data_Cb)(void *data, int fd);

void manager_init(void);
int manager_run(void);
void manager_stop(void);
void manager_register_fd(int fd, Fd_Data_Cb cb, void *data);
void manager_unregister_fd(int fd);

#endif