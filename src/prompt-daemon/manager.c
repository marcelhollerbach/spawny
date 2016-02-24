#include "manager.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

typedef struct {
    Fd_Data_Cb cb;
    Fd_Data data;
} Fd_Register;

#define MAX_BUFFER_SIZE 2048

static int fd_number = 0;
static Fd_Register *cbs = NULL; //array of length fd_number

static int stop = 0;

void
manager_init(void) {

}

static void
_error_handling(result) {
    perror("manager failed, reason :");
}

static void
_read_available(Fd_Register *fd)
{
   uint8_t buffer[MAX_BUFFER_SIZE];
   int len = 0;

   //read
   len = read(fd->data.fd, buffer, sizeof(buffer));

   if (len < 0) {
       //check for errors
       perror("Reading greeter failed");
   }else if (len == 0) {
       //check if empty
       printf("Fd %d disappeared\n", fd->data.fd);
       //the fd is at the end
       manager_unregister_fd(fd->data.fd);
       return;
   }else {
       //call callback!
       fd->cb(&fd->data, buffer, len);
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

        //check for error
        if (result == -1) {
            _error_handling(result);
            return 0;
        } else if (result >= 0) {
            //go throuw the set and check if the fd is set
            for(int i = 0; i < fd_number; i ++) {
                int fd = cbs[i].data.fd;
                //check if this fd is set
                if (FD_ISSET(fd, &fds_read)) {
                    _read_available(&cbs[i]);
                    //
                    //fds could be deleted as a result of the callback

                    //check if we are the end
                    if (i == fd_number) break;
                    //if the fd is different than the one at the start the fd is deleted
                    if (cbs[i].data.fd != fd) {
                        i --;
                        continue;
                    }

                }
                if (FD_ISSET(fd, &fds_error)) {
                    //FIXME ugly
                    printf("Error on %d\n", fd);
                    manager_unregister_fd(fd);
                }
            }
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
          break;
    }

    if (field_index == fd_number)
      return; //not found

    if (field_index < fd_number - 1) {
        //we are not the last item we need to swap
        cbs[field_index].cb = cbs[fd_number -1].cb;
        cbs[field_index].data.fd = cbs[fd_number -1].data.fd;
    }

    //one less fd
    fd_number --;

    //this will cut of the last valid item
    cbs = realloc(cbs, fd_number*sizeof(Fd_Register));
}
