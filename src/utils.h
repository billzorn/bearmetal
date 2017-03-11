// local logic

int  xbind_socket(char *port);
void xnonblock_socket(int sockfd);
void xepoll_addfd(int epfd, int fd, uint32_t events);

// other helpful methods

void xlisten(int sockfd, int backlog);
int  xepoll_create1(int flags);
