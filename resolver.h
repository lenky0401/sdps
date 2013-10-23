
#ifndef __RESOLVER_H__
#define __RESOLVER_H__

struct defend_dns
{
    int ipv4;
    int count;
    time_t last_time;
};

void dns_accept_request(struct my_epoll_data *epoll_data);
void dns_request(struct my_epoll_data *epoll_data);
void dns_response(struct my_epoll_data *epoll_data);

#endif /* __RESOLVER_H__ */
