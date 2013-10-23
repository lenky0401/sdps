#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#define signal_helper(n)     SIG##n
#define signal_value(n)      signal_helper(n)
#define SHUTDOWN_SIGNAL      QUIT
#define TERMINATE_SIGNAL     TERM
#define NOACCEPT_SIGNAL      WINCH
#define RECONFIGURE_SIGNAL   HUP

#if (LINUXTHREADS)
#define REOPEN_SIGNAL        INFO
#define CHANGEBIN_SIGNAL     XCPU
#else
#define REOPEN_SIGNAL        USR1
#define CHANGEBIN_SIGNAL     USR2
#endif

typedef struct {
    int     signo;
    const char   *signame;
    const char   *name;
    void  (*handler)(int signo);
} signal_t;


int init_signals(void);

#endif // __SIGNAL_H__
