#ifndef SERVER_H
#define SERVER_H

int server_init(void);
void server_shutdown(void);
void server_spawnservice_feedback(int success, const char *message, int fd);
int prefetch_server_socket(void);

#endif
