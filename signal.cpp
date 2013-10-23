
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "signal.h"
#include "main.h"

#define value_helper(n)   #n
#define value(n)          value_helper(n)

void signal_handler(int signo);

signal_t  signals[] = {
    { signal_value(RECONFIGURE_SIGNAL),
      "SIG" value(RECONFIGURE_SIGNAL),
      "reload",
      signal_handler },

    { signal_value(REOPEN_SIGNAL),
      "SIG" value(REOPEN_SIGNAL),
      "reopen",
      signal_handler },

    { signal_value(NOACCEPT_SIGNAL),
      "SIG" value(NOACCEPT_SIGNAL),
      "",
      signal_handler },

    { signal_value(TERMINATE_SIGNAL),
      "SIG" value(TERMINATE_SIGNAL),
      "stop",
      signal_handler },

    { signal_value(SHUTDOWN_SIGNAL),
      "SIG" value(SHUTDOWN_SIGNAL),
      "quit",
      signal_handler },

    { signal_value(CHANGEBIN_SIGNAL),
      "SIG" value(CHANGEBIN_SIGNAL),
      "",
      signal_handler },

    { SIGALRM, "SIGALRM", "", signal_handler },

    { SIGINT, "SIGINT", "", signal_handler },

    { SIGIO, "SIGIO", "", signal_handler },

    { SIGCHLD, "SIGCHLD", "", signal_handler },

    { SIGSYS, "SIGSYS, SIG_IGN", "", SIG_IGN },

    { SIGPIPE, "SIGPIPE, SIG_IGN", "", SIG_IGN },

    { 0, NULL, "", NULL }
};


int init_signals(void)
{
    signal_t      *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
            printf("sigaction(%s) failed.\n", sig->signame);
            return -1;
        }
    }

    return 0;
}

void signal_handler(int signo)
{
    int        err;
    signal_t    *sig;

    err = errno;

    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }

    switch (process_type) {

    case PROCESS_MAIN:
        switch (signo) {

        case signal_value(SHUTDOWN_SIGNAL):
            quit = 1;
            break;

        case signal_value(TERMINATE_SIGNAL):
        case SIGINT:
            terminate = 1;
            break;

        case SIGCHLD:
            reap = 1;
            break;
        }

        break;

    case PROCESS_WORKER:
        switch (signo) {

        case signal_value(SHUTDOWN_SIGNAL):
            quit = 1;
            break;

        case signal_value(TERMINATE_SIGNAL):
        case SIGINT:
            terminate = 1;
            break;
        }

        break;
    }

    errno = err;
}


