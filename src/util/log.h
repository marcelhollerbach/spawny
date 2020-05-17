#ifndef LOG_H
#define LOG_H

typedef enum
{
   LOG_TYPE_INF = 0,
   LOG_TYPE_ERR = 1,
} Log_Type;

void
log_init(void);
void
log_to_type(Log_Type type, char *file, int line_number, const char *msg, ...);

#define INF(msg, ...)                                                          \
   log_to_type(LOG_TYPE_INF, __FILE__, __LINE__, msg, ##__VA_ARGS__);
#define ERR(msg, ...)                                                          \
   log_to_type(LOG_TYPE_ERR, __FILE__, __LINE__, msg, ##__VA_ARGS__);

#endif
