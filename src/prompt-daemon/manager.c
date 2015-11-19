#include "manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

typedef struct {
    Fd_Data_Cb cb;
    int fd;
    void *data;
} Fd_Register;

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

int
manager_run(void) {
    while (!stop) {
        int result;
        int max_fd = 0;
        struct timeval timeout;
        fd_set fds_read, fds_error;

        FD_ZERO(&fds_read);
        FD_ZERO(&fds_error);

        for(int i = 0; i < fd_number; i ++) {
            if (cbs[i].fd > max_fd) max_fd = cbs[i].fd;
            FD_SET(cbs[i].fd, &fds_read);
            FD_SET(cbs[i].fd, &fds_error);
        }
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        result = select(max_fd + 1, &fds_read, NULL, &fds_error, &timeout);

        if (result == -1) {
            _error_handling(result);
            return 0;
        } else if (result >= 0) {
            for(int i = 0; i < fd_number; i ++) {
                int fd = cbs[i].fd;
                if (FD_ISSET(fd, &fds_read)) {
                    cbs[i].cb(cbs[i].data, fd);
                }
                if (i == fd_number) break;
                if (cbs[i].fd != fd) {
                    i --;
                    continue;
                }
                if (FD_ISSET(fd, &fds_error)) {
                    //FIXME ugly
                    printf("Error on %d\n", fd);
                    manager_unregister_fd(fd);
                }
                if (i == fd_number) break;
                if (cbs[i].fd != fd) {
                    i --;
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
    cbs[fd_number - 1].data = data;
    cbs[fd_number - 1].fd = fd;
}

void
manager_unregister_fd(int fd) {
    int field_index;

    //search for the fd
    for(field_index = 0; field_index < fd_number; field_index ++) {
        if (cbs[field_index].fd == fd)
          break;
    }

    if (field_index == fd_number)
      return; //not found

    if (field_index < fd_number - 1) {
        //we are not the last item we need to swap
        cbs[field_index].cb = cbs[fd_number -1].cb;
        cbs[field_index].fd = cbs[fd_number -1].fd;
    }

    //one less fd
    fd_number --;

    //this will cut of the last valid item
    cbs = realloc(cbs, fd_number*sizeof(Fd_Register));
}
