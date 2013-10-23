
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <malloc.h>

#include "resolver.h"
#include "epoll.h"
#include "main.h"

//配置上一级DNS服务器ip地址，目前仅第一个生效
const char *dns_addr[64] = {"192.168.1.253", "8.8.8.8", "8.8.4.4", "202.97.224.69", "202.97.224.68"};

int create_server_dns_connect(int *response_fd)
{
    int s;
    struct sockaddr_in srv_sockaddr;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        printf("socket() failed:%s\n", strerror(errno));
        return -1;
    }

    if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == -1) {
        printf("fcntl() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    bzero(&srv_sockaddr, sizeof(srv_sockaddr));
    srv_sockaddr.sin_family = AF_INET;
    srv_sockaddr.sin_port = htons(53);

    if (inet_pton(AF_INET, dns_addr[0], &srv_sockaddr.sin_addr) == -1)
    {
        printf("inet_pton() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    if (connect(s, (struct sockaddr *)&srv_sockaddr, sizeof(srv_sockaddr)) == -1) {
        printf("connect() failed:%s\n", strerror(errno));

        close(s);
        return -1;
    }

    *response_fd = s;
    return 0;
}

#define SOCK_ADDR_LEN 512
#define REQ_DATA_LEN 1500
void dns_accept_request(struct my_epoll_data *epoll_data)
{
    int response_fd;
    int s;
    u_char sa[SOCK_ADDR_LEN];
    socklen_t sock_len;
    char data[REQ_DATA_LEN];
    int data_len;
    struct my_epoll_data *response_data;

    sock_len = SOCK_ADDR_LEN;
    s = accept(epoll_data->request_fd, (struct sockaddr *)sa, &sock_len);
    if (s == -1) {
        printf("accept() failed:%s\n", strerror(errno));
        return;
    }

    if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == -1) {
        printf("fcntl() failed:%s\n", strerror(errno));

        close(s);
        return;
    }

    data_len = read(s, data, REQ_DATA_LEN);
    if (data_len == -1) {
        close(s);
        return;
    }

    if (create_server_dns_connect(&response_fd) == -1) {
        close(s);
        return;
    }

    if (write(response_fd, data, data_len) != data_len) {
        printf("write() failed:%d\n", errno);
        close(s);
        close(response_fd);
        return;
    }

    response_data = (struct my_epoll_data *)malloc(sizeof(struct my_epoll_data));
    if (response_data == NULL) {
        printf("malloc() failed:%d\n", errno);
        close(s);
        close(response_fd);
        return;
    }
    response_data->flag = DNS_SERVER_SOCKET;
    response_data->dns_type = SERVER_SOCKET_TCP;
    response_data->request_fd = s;
    response_data->response_fd = response_fd;

    if (epoll_add_dns_fd(response_fd, response_data) == -1) {
        printf("epoll_add_request_fd() failed:%s\n", strerror(errno));

        free(response_data);
        close(s);
        close(response_fd);
        return;
    }

}

void dns_request(struct my_epoll_data *epoll_data)
{
    int response_fd;
    char data[REQ_DATA_LEN];
    int data_len;
    socklen_t len = sizeof(epoll_data->client_addr);
    struct my_epoll_data *response_data;

    if (create_server_dns_connect(&response_fd) == -1) {
        return;
    }

    response_data = (struct my_epoll_data *)malloc(sizeof(struct my_epoll_data));
    if (response_data == NULL) {
        printf("malloc() failed:%d\n", errno);
        return;
    }
    response_data->flag = DNS_SERVER_SOCKET;
    response_data->dns_type = SERVER_SOCKET_UDP;
    response_data->request_fd = epoll_data->request_fd;
    response_data->response_fd = response_fd;

    data_len = recvfrom(epoll_data->request_fd, data, REQ_DATA_LEN - 1, 0,
            (struct sockaddr*)&(response_data->client_addr), &len);

    if (write(response_fd, data, data_len) != data_len) {
        free(response_data);
        close(response_fd);
        return;
    }

    if (epoll_add_dns_fd(response_fd, response_data) == -1) {
        printf("epoll_add_request_fd() failed:%s\n", strerror(errno));
        free(response_data);
        close(response_fd);
        return;
    }
}

void dns_response(struct my_epoll_data *epoll_data)
{
    char data[REQ_DATA_LEN];
    int data_len;

    data_len = read(epoll_data->response_fd, data, REQ_DATA_LEN);
    if (data_len != -1) {
        if (epoll_data->dns_type == SERVER_SOCKET_TCP) {
            write(epoll_data->request_fd, data, data_len);
        } else {
            sendto(epoll_data->request_fd, data, data_len, 0,
                    (struct sockaddr*)&(epoll_data->client_addr), sizeof(epoll_data->client_addr));
        }
    }

    epoll_del_fd(epoll_data->response_fd, epoll_data);
    close(epoll_data->response_fd);
    if (epoll_data->dns_type == SERVER_SOCKET_TCP) {
        close(epoll_data->request_fd);
    }
}

