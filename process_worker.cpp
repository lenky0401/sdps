
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "process_worker.h"
#include "main.h"
#include "epoll.h"

void worker_init(void)
{
    sigset_t set;
    sigemptyset(&set);

    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
        printf("sigprocmask() failed:%d", errno);
    }

    epoll_init();
    epoll_add_server_fd(listen_socket[0], SERVER_SOCKET_TCP);
    epoll_add_server_fd(listen_socket[1], SERVER_SOCKET_UDP);

}

void worker_loop(void)
{
    process_type = PROCESS_WORKER;

    worker_init();

    for ( ;; ) {

        epoll_process_events(500);

        if (terminate || quit) {
            exit(0);
        }
    }

}
