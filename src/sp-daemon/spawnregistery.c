#include "main.h"

typedef struct {
   pid_t pid;
   void(*handler)(void *data, int status, pid_t pid);
   void *data;
} Waiting;

static Waiting *pending_process = NULL;
static int len = 0;

void
spawnregistery_listen(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data)
{
   len ++;
   pending_process = realloc(pending_process, len * sizeof(Waiting));

   pending_process[len - 1].pid = pid;
   pending_process[len - 1].handler = handler;
   pending_process[len - 1].data = data;
}

void
spawnregistery_unlisten(pid_t pid)
{
   char search_start = 1;
   for (int i = 0; i < len; i ++)
     {
        switch(search_start){
            case 1:
              if (pending_process[i].pid == pid)
                search_start = 0;
            break;
            case 0:
              pending_process[i - 1] = pending_process[i];
            break;
        }
     }
   if (search_start == 1) return;

   len --;
   pending_process = realloc(pending_process, len * sizeof(Waiting));
}

void
spawnregistery_eval(void)
{
   int status;
   pid_t pid;

   while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
       printf("Looks like %d has terminated\n", pid);
       for(int i = 0; i < len; i++)
         {
            if (pending_process[i].pid == pid)
              {
                 pending_process[i].handler(pending_process[i].data, status, pid);
                 spawnregistery_unlisten(pid);

                 break;
              }
         }
    }

}
