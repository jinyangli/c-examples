/*
** echo-server.c -- an echo server demo, code adapted from 
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#clientserver
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
#include <assert.h>
#include <pthread.h>

void *
handle_connection(void *arg)
{
	int fd = (int)arg;
	
	char buf[1000];
	int s;
	while ((s = read(fd, buf, 1000)) > 0) {
		printf("echo server: echoing %d bytes received from fd=%d\n", s, fd);
		int rs = write(fd, buf, s);
		assert(rs == s);
	}
	close(fd);
	printf("echo server: fd=%d closed\n", fd);
}

int 
main(int argc, char **argv)
{

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // step 1: perform getaddrinfo to fill socket data structure
    struct addrinfo *servinfo;
    assert(getaddrinfo(NULL, "8888", &hints, &servinfo) == 0);

    // loop through all the results and bind to the first we can
    int sockfd = -1;
    for(struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
	//step 2: create socket file descriptor    
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

	//allows me to rebind to the same port after restart immediately
    	int yes=1;
        assert(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == 0);

	//step 3: bind sockfd to the specified address & port
        assert(bind(sockfd, p->ai_addr, p->ai_addrlen) == 0);

        break;
    }

    freeaddrinfo(servinfo); // no use for servinfo anymore

    assert(sockfd >= 0);

    //step 4: mark sockfd as allowing for incoming connections, with a wait queue size of 10
    assert(listen(sockfd, 10) == 0);

    printf("echo server: waiting for connections...\n");

    //step 5: do the accept loop
    while(1) {  
    	struct sockaddr_in remote_addr; // connector's address information
        socklen_t sin_size = sizeof(remote_addr);
        int conn_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size);
        assert(conn_fd >= 0);

	char buf[100];
        inet_ntop(AF_INET, &remote_addr.sin_addr, buf, sizeof(buf));
        printf("echo server: accepted connection from %s\n", buf);

	/*
	// handle connection in a separate process
        if (!fork()) { // child 
            close(sockfd); // child doesn't need the listener
	    handle_connection((void *)conn_fd);
            exit(0);
        } else { //parent
	    close(conn_fd); //parent does not the connection's fd 
	}
	*/
	pthread_t th;
	pthread_create(&th, NULL, handle_connection, (void *)conn_fd);
    }
    return 0;
}

