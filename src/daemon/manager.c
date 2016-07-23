#include "manager.h"

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

#define MAX_BUFFER_SIZE 2048

static int fd_number = 0;
static Fd_Register *cbs = NULL; //array of length fd_number

static int stop = 0;
static int changes = 0;
static int readout = 0;

void
manager_init(void) {

}

static void
_error_handling(result) {
    perror("manager failed, reason :");
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
   printf("Error on %d\n", reg->data.fd);
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

static void
_clear_objects(void)
{
   int result = 0;
   int mode = 0;
   for (int i = 0; i < fd_number; i++)
     {
        if (mode == 0 && !cbs[i].cb)
          {
             result = 1;
             mode = 1;
          }
        else if (mode == 1)
          {
             cbs[i - 1] = cbs[i];
          }
     }
   if (result)
     {
        fd_number --;
        cbs = realloc(cbs, fd_number*sizeof(Fd_Register));
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
        for(int i = 0; i < fd_number; i ++) {
            if (cbs[i].data.fd > max_fd) max_fd = cbs[i].data.fd;
            FD_SET(cbs[i].data.fd, &fds_read);
            FD_SET(cbs[i].data.fd, &fds_error);
        }
        //set timeout to one second
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        //read the fds
        result = select(max_fd + 1, &fds_read, NULL, &fds_error, &timeout);

        readout = 1;

        //check for error
        if (result == -1) {
            _error_handling(result);
            readout = 0;
            return 0;
        } else if (result >= 0) {
            //look for errors fd´s
            for(int i = 0; i < fd_number; i ++) {
               _fd_set_check(&cbs[i], &fds_error, _error_available, i);
            }

            //look for read fd´s
            for(int i = 0; i < fd_number; i ++) {
               _fd_set_check(&cbs[i], &fds_read, _read_available, i);
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
    //we have a new fd
    fd_number ++;

    cbs = realloc(cbs, fd_number*sizeof(Fd_Register));
    cbs[fd_number - 1].cb = cb;
    cbs[fd_number - 1].data.data = data;
    cbs[fd_number - 1].data.fd = fd;
}


void
manager_unregister_fd(int fd) {
    int field_index;

    //search for the fd
    for(field_index = 0; field_index < fd_number; field_index ++) {
        if (cbs[field_index].data.fd == fd)
          {
             cbs[field_index].cb = NULL;
          }
    }

    if (!readout)
      _clear_objects();
    else
      changes ++;
}
