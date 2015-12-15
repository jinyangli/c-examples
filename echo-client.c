/*
** echo-client.c -- an echo client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>

int 
main(int argc, char **argv)
{

    if (argc !=3) {
	    printf("usage: ./a.out <server> <echo message>\n");
	    exit(1);
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    /* step 1: perform getaddrinfo to fill socket data structure*/
    struct addrinfo *servinfo;
    assert(getaddrinfo(argv[1], "8888", &hints, &servinfo) == 0);

    // loop through all the results and bind to the first we can
    int conn_fd = -1;
    for(struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
	//create socket file descriptor    
        if ((conn_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

	//connect to server
        assert(connect(conn_fd, p->ai_addr, p->ai_addrlen) == 0);
        break;
    }

    freeaddrinfo(servinfo); // no use for servinfo anymore

    assert(conn_fd >= 0);

    int len = strlen(argv[2])+1;
    int r = write(conn_fd, argv[2], len);
    printf("echo client: sending %s to server %s\n", argv[2], argv[1]);

    char *buf = malloc(len);
    buf[len-1] = '\0';
    r = read(conn_fd, buf, len);
    printf("echo client: received %s from server %s\n", buf, argv[1]);

}
