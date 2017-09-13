#include "test.h"
#include <stdio.h>
#include <unistd.h>
#include "../sp-daemon/manager.h"

static void
_break_cond(int i)
{
    if (i > 1)
      manager_stop();
}

static void
_cb(Fd_Data *data UNUSED, int fd UNUSED)
{
   int *i = data->data;

   manager_unregister_fd(data->fd);

   (*i)++;
   _break_cond(*i);
}

START_TEST(test_fd_select)
{
    char msg[] = "Hello world";
    int fd_ready_to_read = 0;
    int fds_a[2];
    int fds_b[2];
    pid_t pid;

    manager_init();

    if (pipe(fds_a) < 0 || pipe(fds_b) < 0)
      ck_abort_msg("pipe creation failed");


    pid = fork();

    if (pid == -1)
      ck_abort_msg("fork failed");
    if (pid == 0) {
        close(fds_a[0]);
        close(fds_b[0]);
        if (write(fds_a[1], msg, strlen(msg) + 1) != (long)strlen(msg) + 1 ||
            write(fds_b[1], msg, strlen(msg) + 1) != (long)strlen(msg) + 1) {
            ck_abort_msg("writing to pipe failed");
        }

        close(fds_a[1]);
        close(fds_b[1]);
        exit(0);
    }
    close(fds_a[1]);
    close(fds_b[1]);
    manager_register_fd(fds_a[0], _cb, &fd_ready_to_read);
    manager_register_fd(fds_b[0], _cb, &fd_ready_to_read);
    ck_assert_int_eq(manager_run(), 1);
    close(fds_a[0]);
    close(fds_b[0]);
    ck_assert_int_eq(fd_ready_to_read, 2);
}
END_TEST

void manager_suite(Suite *s)
{
    TCase *tc_core;

    /* Core test case */
    tc_core = tcase_create("fd select");

    tcase_add_test(tc_core, test_fd_select);
    suite_add_tcase(s, tc_core);
}

