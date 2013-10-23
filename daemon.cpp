
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "daemon.h"

int daemon()
{
    int  fd;

    switch (fork()) {
    case -1:
        printf("fork() failed:%d\n", errno);
        return -1;

    case 0:
        break;

    default:
        exit(0);
    }

    if (setsid() == -1) {
		printf("setsid() failed:%d\n", errno);
        return -1;
    }

    umask(0);

    fd = open("/tmp/defenddns.log", O_CREAT|O_RDWR, 0755);
    if (fd == -1) {
		printf("open(\"/tmp/defenddns.log\") failed:%d\n", errno);
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
		printf("dup2(STDIN) failed:%d\n", errno);
        return -1;
        
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
		printf("dup2(STDOUT) failed:%d\n", errno);
        return -1;
		
    }

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
			printf("close() failed:%d\n", errno);
        	return -1;
        }
    }

    return 0;
}
