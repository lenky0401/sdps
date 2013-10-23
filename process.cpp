
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "signal.h"
#include "process.h"
#include "process_worker.h"
#include "main.h"
#include "conf.h"

static void start_worker(int n)
{
    int i;

    for (i = 0; i < n; i++) {

        switch (fork()) {

        case -1:
            printf("fork() failed:%d", errno);
            return;

        case 0:
            worker_loop();
            break;

        default:
            break;
        }
    }
}

void main_loop()
{
    sigset_t           set;

    process_type = PROCESS_MAIN;

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
    sigaddset(&set, signal_value(RECONFIGURE_SIGNAL));
    sigaddset(&set, signal_value(REOPEN_SIGNAL));
    sigaddset(&set, signal_value(NOACCEPT_SIGNAL));
    sigaddset(&set, signal_value(TERMINATE_SIGNAL));
    sigaddset(&set, signal_value(SHUTDOWN_SIGNAL));
    sigaddset(&set, signal_value(CHANGEBIN_SIGNAL));

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        printf("sigprocmask() failed:%d\n", errno);
    }
    sigemptyset(&set);

    start_worker(WORKER_NUM);

    for ( ;; ) {
        sigsuspend(&set);

        if (reap) {
            reap = 0;
            start_worker(1);
        }

        if (terminate || quit) {
            exit(0);
        }

    }
}
