#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#include "settings.h"
#include "utils.h"

static inline void server_accept(int accept_sockfd, int epfd) {

}

static inline void server_respond(int fd, char *buf, size_t bufsize) {

}

int main(int argc, char *argv[]) {
    int accept_sockfd;
    int server_epfd;
    int x, running;

    struct epoll_event events[BEARMETAL_DEFAULT_MAXEVENTS];
    char readbuf[BEARMETAL_DEFAULT_READBUF];

    // ignore command line arguments
    (void)argc;
    (void)argv;

    accept_sockfd = xbind_socket(BEARMETAL_DEFAULT_PORT);
    xnonblock_socket(accept_sockfd);
    xlisten(accept_sockfd, BEARMETAL_DEFAULT_BACKLOG);

    server_epfd = xepoll_create1(0); // the only flag is EPOLL_CLOEXEC, revist if we do MT
    xepoll_addfd(server_epfd, STDIN_FILENO, EPOLLIN | EPOLLET);
    xepoll_addfd(server_epfd, accept_sockfd, EPOLLIN | EPOLLET);

    running = 1;
    while (running) {
	int nfds, i;
	
	nfds = epoll_wait(server_epfd, events, BEARMETAL_DEFAULT_MAXEVENTS, -1);
	if (nfds == -1) {
	    if (errno == EINTR) {
		// this might happen if the server is stopped, and is probably harmless
		perror("ignoring: epoll_wait");
	    } else {
		// all other errors are fatal
		perror("epoll_wait");
		exit(EXIT_FAILURE);
	    }
	}

	for (i = 0; i < nfds; ++i) {
	    if ((events[i].events & EPOLLERR) ||
		(events[i].events & EPOLLHUP) ||
		(!(events[i].events & EPOLLIN)))
	    {
		fprintf(stderr, "epoll error on fd %d\n", events[i].data.fd);
		x = close(events[i].data.fd);
		if (x != 0) {
		    perror("ignoring: close");
		}
	    } else if (events[i].data.fd == STDIN_FILENO) {
		ssize_t n, total;

		n = read(events[i].data.fd, readbuf, sizeof(readbuf));
		if (n == -1) {
		    perror("ignoring: read");
		}
		
		fprintf(stdout, "Main control: read returned %ld.\n", n);
		if (n <= 0) {
		    fprintf(stdout, "Server exiting.\n");
		    running = 0;
		} else {
		    x = write(STDOUT_FILENO, readbuf, n);
		    if (x == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		    }

		    total = n;
		    while (0 <= n && sizeof(readbuf) <= (size_t)n) {
			n = read(events[i].data.fd, readbuf, sizeof(readbuf));
			if (n == -1) {
			    perror("ignoring: read");
			} else {
			    total += n;
			}
		    }

		    fprintf(stdout, "up to %d bytes shown, read %ld total.\n",
			    BEARMETAL_DEFAULT_READBUF, total);
		}
	    } else if (events[i].data.fd == accept_sockfd) {
		fprintf(stdout, "placeholder: accept\n");
	    } else {
		fprintf(stdout, "placeholder: respond\n");
	    }
	}
    }

    close(accept_sockfd);
    return EXIT_SUCCESS;
}
