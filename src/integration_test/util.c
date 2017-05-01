#define _DEFAULT_SOURCE
#include "test.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include <signal.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

static pid_t daemon_pid = 0;

static char *directories[] = {
  "/lib/", "/lib64", NULL
};

#define FULL_PATH(path) \
   char buf[strlen(SP_CHROOT_DIRECTORY) + 1 + strlen(directory) + 1]; \
   snprintf(buf, sizeof(buf), "%s%s", SP_CHROOT_DIRECTORY, path);

static bool
_link_in(const char *directory)
{
   FULL_PATH(directory)

   if (!!access(buf, W_OK | R_OK)) {
        if (!!mkdir(buf, S_IRWXG | S_IRWXU | S_IRWXO)) {
          perror("mkdir failed:");
          return false;
        }
     }

   if (!!mount(directory, buf, NULL, MS_BIND, NULL)) {
      perror("mount failed:");
      return false;
   }
   return true;
}

static void
_link_out(const char *directory)
{
   //we dont need to append to the SP_CHROOT_DIRECTORY, since this here is chrooted
   if (!!umount2(directory, MNT_FORCE)) {
      perror("umount failed:");
   }
}

bool prepare(void)
{
   for (int i = 0; directories[i]; ++i) {
     if (!_link_in(directories[i]))
       return false;
   }

   //change directory too chroot
   if (!!chdir(SP_CHROOT_DIRECTORY)) {
     perror("Failed to change current working directory");
     return false;
   }

   //change root directory
   if (!!chroot(SP_CHROOT_DIRECTORY)) {
     perror("Error chrooting");
     return false;
   }
   return true;
}

bool teardown(void)
{
   for (int i = 0; directories[i]; ++i) {
     _link_out(directories[i]);
   }

   return true;
}

void daemon_start(void)
{
  if (!!daemon_pid){
    printf("ERROR ERROR\n");
  }
  daemon_pid = fork();

  if (daemon_pid == 0) {
     //child
     char* args[] = {"sp-daemon", "-d", NULL};
                     //preload our systemd stuff
     char* envp[] = {"LD_PRELOAD="SP_PRELOAD_SYSTEMD,
                     //load those places
                     "LD_LIBRARY_PATH=/lib/:"PACKAGE_LIB_DIR,
                     NULL};

     execve(SP_DAEMON, args, envp);
     perror("Error spawning daemon!");
     exit(-1);
  } else if (daemon_pid == -1) {
     //error
  } else {
    //this should be enough for the daemon to start
    sleep(2);
    printf("Spawned daemon with pid:%d\n", daemon_pid);
  }
}

void daemon_stop(void)
{
   if (!daemon_pid) return;

   kill(daemon_pid, SIGTERM);
   daemon_pid = 0;
}
