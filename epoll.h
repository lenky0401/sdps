#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <netinet/in.h>
#include <sys/socket.h>

int epoll_init();
void epoll_done();
int epoll_add_server_fd(int fd, int flag);
int epoll_add_dns_fd(int fd, struct my_epoll_data *data);
int epoll_del_fd(int fd, struct my_epoll_data *data);
int epoll_process_events(int timer);

struct my_epoll_data
{
    int flag;
    int dns_type;
    int request_fd;
    int response_fd;
    struct sockaddr_in client_addr;
};

#endif // __EPOLL_H__
