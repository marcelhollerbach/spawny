#include "main.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    Fd_Data_Cb cb;
    Fd_Data data;
} Fd_Register;

ARRAY_API(Fd_Register)

#define MAX_BUFFER_SIZE 2048

static Array* fds;

static int stop = 0;
static int changes = 0;
static int readout = 0;

void
manager_init(void) {
   fds = array_Fd_Register_new();
}

static void
_read_available(Fd_Register *reg, int fd)
{
    //call callback!
    reg->cb(&reg->data, fd);
}

static void
_error_available(Fd_Register *reg, int fd)
{
   ERR("Error on %d", reg->data.fd);
   manager_unregister_fd(reg->data.fd);
}

static void
_fd_set_check(Fd_Register *reg, fd_set *set, void (*handle)(Fd_Register *reg, int fd), int i)
{
   //check if set
   if (FD_ISSET(reg->data.fd, set)) {
      handle(reg, reg->data.fd);
   }
}

static int
_errno_ignore(void) {
   if (errno == EINTR) return 1;
   return 0;
}

static void
_clear_objects(void) {
   for(int i = 0; i < array_len_get(fds); i++){
      Fd_Register *reg = array_Fd_Register_get(fds, i);
      if (!reg->cb) {
        array_Fd_Register_del(fds, i);
        return;
      }
   }
}

int
manager_run(void) {
    while (!stop) {
        int result;
        int max_fd = 0;
        struct timeval timeout;
        fd_set fds_read, fds_error;

        //null out the fd sets
        FD_ZERO(&fds_read);
        FD_ZERO(&fds_error);

        //init sets with new fds
        for(int i = 0; i < array_len_get(fds); i ++) {
            Fd_Register *reg;

            reg = array_Fd_Register_get(fds, i);
            if (reg->data.fd > max_fd) max_fd = reg->data.fd;
            FD_SET(reg->data.fd, &fds_read);
            FD_SET(reg->data.fd, &fds_error);
        }
        //set timeout to one second
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        //read the fds
        result = select(max_fd + 1, &fds_read, NULL, &fds_error, &timeout);

        readout = 1;

        //check for error
        if (result == -1 && !_errno_ignore()) {
            perror("Manager failed, reason");
            readout = 0;
            return 0;
        } else if (result >= 0) {
            //look for errors fd´s
            for(int i = 0; i < array_len_get(fds); i ++) {
               _fd_set_check(array_Fd_Register_get(fds, i), &fds_error, _error_available, i);
            }

            //look for read fd´s
            for(int i = 0; i < array_len_get(fds); i ++) {
               _fd_set_check(array_Fd_Register_get(fds, i), &fds_read, _read_available, i);
            }
       }
       readout = 0;

       //flush out the changes
       for(; changes > 0; changes --)
         {
            _clear_objects();
         }
    }
    return 1;
}

void
manager_stop(void) {
    stop = 1;
}

void
manager_register_fd(int fd, Fd_Data_Cb cb, void *data) {
   Fd_Register *r;

   r = array_Fd_Register_add(fds);

   r->cb = cb;
   r->data.data = data;
   r->data.fd = fd;
}


void
manager_unregister_fd(int fd) {

    //search for the fd
    for(int field_index = 0; field_index < array_len_get(fds); field_index ++) {
        Fd_Register *reg = array_Fd_Register_get(fds, field_index);
        if (reg->data.fd == fd)
          {
             reg->cb = NULL;
          }
    }

    if (!readout)
      _clear_objects();
    else
      changes ++;
}
