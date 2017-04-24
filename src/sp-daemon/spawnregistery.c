#include "main.h"
#include <string.h>

typedef struct {
   pid_t pid;
   void(*handler)(void *data, int status, pid_t pid);
   void *data;
} Waiting;

ARRAY_API(Waiting)

static Array* pending_process;

static bool eval_mode;

void
spawnregistery_init(void)
{
   pending_process = array_Waiting_new();
}

void spawnregistery_shutdown(void)
{
  array_free(pending_process);
}

void
spawnregistery_listen(pid_t pid, void(*handler)(void *data, int status, pid_t pid), void *data)
{
   Waiting *w;

   w = array_Waiting_add(pending_process);

   w->pid = pid;
   w->handler = handler;
   w->data = data;
}

bool
_spawnregistery_cleanup_step(void)
{
   for (int i = 0; i < array_len_get(pending_process); i ++)
     {
        Waiting *w = array_Waiting_get(pending_process, i);
        if (w->handler) continue;

        array_Waiting_del(pending_process, i);
        return true;
     }

   return false;
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
   for (int i = 0; i < array_len_get(pending_process); i ++)
     {
        Waiting *w = array_Waiting_get(pending_process, i);
        if (w->pid == pid && w->handler == handler && w->data == data )
          {
             memset(w, 0, sizeof(Waiting));
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
       INF("Looks like %d has terminated", pid);
       for(int i = 0; i < array_len_get(pending_process); i++)
         {
            Waiting *w = array_Waiting_get(pending_process, i);
            if (w->pid == pid)
              {
                 void(*handler)(void *data, int status, pid_t pid) = w->handler;
                 void *data = w->data;

                 w->handler(w->data, status, pid);
                 spawnregistery_unlisten(pid, handler, data);
              }
         }
    }

   eval_mode = false;

   _spawnregistery_cleanup();
}
