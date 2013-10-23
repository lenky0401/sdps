
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "epoll.h"
#include "main.h"
#include "resolver.h"

static int ep = -1;
static struct epoll_event *event_list;
static unsigned int nevents;

int epoll_init()
{
    ep = epoll_create(1024);
    if (ep == -1) {
        printf("epoll_create() failed:%d\n", errno);
        return -1;
    }

    nevents = 1024;
    event_list = (struct epoll_event *)malloc(sizeof(struct epoll_event) * nevents);
    if (event_list == NULL) {
        printf("malloc() failed:%d\n", errno);
        return -1;
    }

    return 0;
}

void epoll_done()
{
    if (close(ep) == -1) {
        printf("close() failed:%d\n", errno);
    }
    ep = -1;

    free(event_list);
    event_list = NULL;
    nevents = 0;
}

int epoll_add_server_fd(int fd, int flag)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;

    struct my_epoll_data *data = (struct my_epoll_data *)malloc(sizeof(struct my_epoll_data));
    if (data == NULL) {
        printf("malloc() failed:%d\n", errno);
        return -1;
    }
    data->flag = flag;
    data->request_fd = fd;
    data->response_fd = -1;

    ev.data.ptr = (void *)data;

    if (epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl() failed:%d\n", errno);
        return -1;
    }

    return 0;
}

int epoll_add_dns_fd(int fd, struct my_epoll_data *data)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void *)data;

    if (epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl() failed:%d\n", errno);
        return -1;
    }

    return 0;
}

int epoll_del_fd(int fd, struct my_epoll_data *data)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void *)data;

    if (epoll_ctl(ep, EPOLL_CTL_DEL, fd, &ev) == -1) {
        printf("epoll_ctl() failed:%d\n", errno);
        return -1;
    }

    free(data);

    return 0;
}

int epoll_process_events(int timer)
{
    int events;
    int err;
    int flag;
    int i;

    events = epoll_wait(ep, event_list, (int) nevents, timer);

    err = (events == -1) ? errno : 0;

    if (err) {
        if (err != EINTR) {
            printf("epoll_wait() failed:%d\n", errno);
            return -1;
        }
        return 0;
    }

    if (events != 0) {
        for (i = 0; i < events; i++) {
            flag = ((struct my_epoll_data *)event_list[i].data.ptr)->flag;
            if (flag == SERVER_SOCKET_TCP) {
                dns_accept_request((struct my_epoll_data *)event_list[i].data.ptr);

            } else if (flag == SERVER_SOCKET_UDP) {
                dns_request((struct my_epoll_data *)event_list[i].data.ptr);

            } else if (flag == DNS_SERVER_SOCKET) {
                dns_response((struct my_epoll_data *)event_list[i].data.ptr);
            }
        }
    }

    return 0;
}
