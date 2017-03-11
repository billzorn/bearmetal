#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "settings.h"
#include "utils.h"

// local logic

static int bind_socket(char *port) {
    struct addrinfo hints;
    struct addrinfo *results, *r;
    int x, sockfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = BEARMETAL_DEFAULT_AI_FAMILY;
    hints.ai_socktype = BEARMETAL_DEFAULT_AI_SOCKTYPE;
    hints.ai_flags    = BEARMETAL_DEFAULT_AI_FLAGS;

    x = getaddrinfo(NULL, port, &hints, &results);
    if (x != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(x));
	return -1;
    }

    for (r = results; r != NULL; r = r->ai_next) {
	sockfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (sockfd == -1) {
	    perror("ignoring: socket");
	} else {
	    x = bind(sockfd, r->ai_addr, r->ai_addrlen);
	    if (x != 0) {
		perror("ignoring: bind");
		x = close(sockfd);
		if (x != 0) {
		    perror("ignoring: close");
		}
	    } else {
		// success
		break;
	    }
	}
    }

    freeaddrinfo(results);

    if (r == NULL) {
	fprintf(stderr, "bind_socket: Unable to bind\n");
	return -1;
    } else {
	return sockfd;
    }
}

static int nonblock_socket(int sockfd) {
    int flags, x;
    
    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
	return -1;
    }

    flags |= O_NONBLOCK;
    x = fcntl(sockfd, F_SETFL, flags);
    if (x == -1) {
	return -1;
    } else {
	return 0;
    }
}

static int epoll_addfd(int epfd, int fd, uint32_t events) {
    struct epoll_event event;
    int x;
    
    event.data.fd = fd;
    event.events = events;
    x = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    if (x == -1) {
	return -1;
    } else {
	return 0;
    }
}

// interface to local logic

int xbind_socket(char *port) {
    int sockfd;
    sockfd = bind_socket(port);
    if (sockfd == -1) {
	// error has already been reported
	exit(EXIT_FAILURE);
    } else {
	return sockfd;
    }
}

void xnonblock_socket(int sockfd) {
    int x;
    x = nonblock_socket(sockfd);
    if (x != 0) {
	// nonblock_socket returns -1 only after a fcntl error
	perror("nonblock_socket: fcntl");
	exit(EXIT_FAILURE);
    } else {
	return;
    }
}

void xepoll_addfd(int epfd, int fd, uint32_t events) {
    int x;
    x = epoll_addfd(epfd, fd, events);
    if (x != 0) {
	// epoll_addfd returns -1 only after an epoll_ctl error
	perror("epoll_addfd: epoll_ctl");
	exit(EXIT_FAILURE);
    } else {
	return;
    }
}

// interface to other helpful methods

void xlisten(int sockfd, int backlog) {
    int x;
    x = listen(sockfd, backlog);
    if (x != 0) {
	perror("listen");
	exit(EXIT_FAILURE);
    } else {
	return;
    }
}

int xepoll_create1(int flags) {
    int epfd;
    epfd = epoll_create1(flags);
    if (epfd == -1) {
	perror("epoll_create1");
	exit(EXIT_FAILURE);
    } else {
	return epfd;
    }
}
