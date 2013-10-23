#ifndef __MAIN_H__
#define __MAIN_H__

#define PROCESS_MAIN 0
#define PROCESS_WORKER 1

#define SERVER_SOCKET_TCP 1
#define SERVER_SOCKET_UDP 2
#define DNS_SERVER_SOCKET 4

extern int process_type;
extern int reap;
extern int terminate;
extern int quit;

extern int listen_socket_num;
extern int listen_socket[2];

#endif // __MAIN_H__
