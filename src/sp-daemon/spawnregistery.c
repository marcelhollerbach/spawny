#include "main.h"

typedef struct {
   pid_t pid;
   void(*handler)(void *data, int status, pid_t pid);
   void *data;
} Waiting;

static Waiting *pending_process = NULL;
static int len = 0;
static bool eval_mode;

void
spawnregistery_listen(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data)
{
   len ++;
   pending_process = realloc(pending_process, len * sizeof(Waiting));

   pending_process[len - 1].pid = pid;
   pending_process[len - 1].handler = handler;
   pending_process[len - 1].data = data;
}

bool
_spawnregistery_cleanup_step(void)
{
   bool found = false;

   for (int i = 0; i < len; i ++)
     {
        if (pending_process[i].handler == NULL)
          {
             //we have found a empty handler
             //move all Waiting structs one index higher
             int move_len = 0;
             move_len = len - (i + 1);
             memmove(&pending_process[i], &pending_process[i + 1], move_len * sizeof(Waiting));
             len --;
             pending_process = realloc(pending_process, len * sizeof(Waiting));
             found = true;
             break;
          }
     }

   return found;
}

static void
_spawnregistery_cleanup(void)
{
   while (1) {
      if (!_spawnregistery_cleanup_step()) break;
   }
}

void
spawnregistery_unlisten(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data)
{
   for (int i = 0; i < len; i ++)
     {
        Waiting *w = &pending_process[i];
        if (w->pid == pid && w->handler == handler && w->data == data )
          {
             memset(&pending_process[i], 0, sizeof(Waiting));
          }
     }

   if (!eval_mode)
     {
        _spawnregistery_cleanup();
     }
}

void
spawnregistery_eval(void)
{
   int status;
   pid_t pid;

   eval_mode = true;

   while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
       printf("Looks like %d has terminated\n", pid);
       for(int i = 0; i < len; i++)
         {
            if (pending_process[i].pid == pid)
              {
                 pending_process[i].handler(pending_process[i].data, status, pid);
                 spawnregistery_unlisten(pid, pending_process[i].handler, pending_process[i].data);
              }
         }
    }

   eval_mode = false;

   _spawnregistery_cleanup();
}
