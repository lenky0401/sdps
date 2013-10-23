
#include <pthread.h>
#include <stdlib.h>

#include "main.h"
#include "daemon.h"
#include "signal.h"
#include "process.h"
#include "listen.h"

int process_type;
int reap;
int terminate;
int quit;
int listen_socket_num = 2;
int listen_socket[2];

int main() {

    if (init_signals())
        return -1;

    if (daemon())
        return -1;

    if (init_listen())
        return -1;

    main_loop();

    return 0;

}
