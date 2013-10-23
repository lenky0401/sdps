
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "listen.h"
#include "main.h"

int add_listen(int port, int type, int idx)
{
    int s;
    int reuseaddr;
    struct sockaddr_in my_sockaddr;

    s = socket(AF_INET, type, 0);
    if (s == -1) {
        printf("socket() failed:%s\n", strerror(errno));
        return -1;
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
            (const void *) &reuseaddr, sizeof(int)) == -1)
    {
        printf("setsockopt() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == -1) {
        printf("fcntl() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    bzero(&my_sockaddr, sizeof(my_sockaddr));
    my_sockaddr.sin_family = AF_INET;
    my_sockaddr.sin_port = htons(port);
    my_sockaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&my_sockaddr, sizeof(struct sockaddr)) == -1) {
        if (errno != EADDRINUSE) {
            printf("bind() failed:%s\n", strerror(errno));

            close(s);
            return -1;
        }
    }

    if (type == SOCK_STREAM && listen(s, 1024) == -1) {
        printf("listen() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    listen_socket[idx] = s;

    return 0;
}

int init_listen(void)
{

    if (add_listen(53, SOCK_STREAM, 0) == -1) {
        printf("add_listen() failed:%d\n", 0);
        return -1;
    }

    if (add_listen(53, SOCK_DGRAM, 1) == -1) {
        printf("add_listen() failed:%d\n", 1);
        return -1;
    }

    return 0;
}


