#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include "friendme.h"

#ifndef PORT
  #define PORT 52792
#endif

static int listenfd;

struct client {
    int fd;
    //stores what is to be written to the client
    char towrite[INPUT_BUFFER_SIZE];
    int inbuf;
    int room;
    char * after;
    struct in_addr ipaddr;
    struct client *next;
} *top = NULL;

static void addclient(int fd, struct in_addr addr)
{
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
	fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
	exit(1);
    }
    printf("Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    // Set client values
    p->inbuf = 0;
    p->after = p->towrite;
    p->room = sizeof(p->towrite);
    memset(p->towrite, '\0', p->room);
    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    top = p;
}

static void removeclient(int fd)
{
    struct client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
	;
    if (*p) {
	struct client *t = (*p)->next;
	printf("Removing client %s\n", inet_ntoa((*p)->ipaddr));
	fflush(stdout);
	free(*p);
	*p = t;
    } else {
	fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
	fflush(stderr);
    }
}

void bindandlisten()  /* bind and listen, abort on error */
{
    struct sockaddr_in r;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
	perror("bind");
	exit(1);
    }

    if (listen(listenfd, 5)) {
	perror("listen");
	exit(1);
    }
}


void newconnection(User ** user_list_ptr)
{
    int fd;
    struct sockaddr_in r;
    socklen_t socklen = sizeof r;
    int len;
    int c;
    char username[MAX_NAME];

    if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
	perror("accept");
    } else {
	printf("connection from %s\n", inet_ntoa(r.sin_addr));
	addclient(fd, r.sin_addr);
	// Send welcome message
	char * greeting = "Welcome! to FriendMe!\nEnter your username: ";
	write(fd, greeting, strlen(greeting));

	/* wait for and get response -- look for first non-whitespace char */
	/* (buf is a good enough size for reads -- it probably will all fit
	 * into one read() for any non-trivial size.) */
	c = -2;  /* (neither a valid char nor the -1 error signal below) */
	while (c == -2) {
	    fd_set fdlist;
	    struct timeval tv;
	    FD_ZERO(&fdlist);
	    FD_SET(fd, &fdlist);
	    tv.tv_sec = 15;
	    tv.tv_usec = 0;
	    if (select(fd+1, &fdlist, NULL, NULL, &tv) != 1) {
		c = -1;
		break;
	    }
	    //read username from client
	    if ((len = read(fd, username, MAX_NAME)) == 0) { 
		c = -1;
	    } else if (len < 0) {
		if (errno != EINTR)
		    c = -1;
	    } else {
			//Create User from username
			username[len] = '\0';
			printf("Username Entered: %s\n", username);
			//call create_user in friends.c
			if(find_user(username, *user_list_ptr) != NULL){
				printf("Welcome back %s!", username);
			} else {
				create_user(username, user_list_ptr);
			}
			/*
			switch (create_user(username, user_list_ptr)) {
            case 1:
				printf("Welcome Back: %s\n", username);
                error("user by this name already exists");
                break;
            case 2:
                error("username is too long");
                break;
			}*/
			/*
			for (i = 0; i < len; i++) {
		    if (isascii(buf[i]) && !isspace(buf[i])) {
			c = buf[i];
			break;
		    }
			*/
			break;
	    }
	}
    }
}

/*
 * Return the index of '\r' in the first inbuf indexes of char * buf. 
 * Return -1 if '\r' is not in char * buf.
 */
int find_network_newline(const char *buf, int inbuf) {
  int i;
  for(i=0; i< inbuf;i++){
	  if(buf[i] == '\r'){
		  return i;
	  }
  }
  return -1; // return the location of '\r' if found
}

void whatsup(struct client *p, User ** user_list_ptr)  /* select() said activity; check it out */
{
	//buff read goes here
    int len = read(p->fd, p->after, p->room);
    if (len > 0) {
		p->inbuf += len;
		int where = find_network_newline(p->towrite, p->inbuf);
		if (where >= 0){
			p->towrite[where] = '\n';
			p->towrite[where+1] ='\0';
			printf("Command Recieved: %s", p->towrite);
			//Run command in friendme
			char *cmd_argv[INPUT_ARG_MAX_NUM];
			int cmd_argc = tokenize(p->towrite, cmd_argv);
			process_args(cmd_argc, cmd_argv, user_list_ptr, p->fd);
			p->inbuf = p->inbuf - where - 2;
			memmove(&(p->towrite), &(p->towrite[where+2]), p->inbuf);
			p->towrite[p->inbuf+1] = '\0';
		}
		p->room = sizeof(p->towrite) - p->inbuf;
		p->after = &(p->towrite[p->inbuf]);
	/* discard (probably more of the "yes!!!!" string) */
    } else if (len == 0) {
	printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
	fflush(stdout);
	removeclient(p->fd);
    } else {
	/* shouldn't happen */
	perror("read");
    }
}

int main() {
    struct client *p;
    User ** user_list = malloc(sizeof(User *));
    *user_list = malloc(sizeof(User));
	*user_list = NULL;
    
    bindandlisten();
    
    while (1) {
	fd_set fdlist;
	int maxfd = listenfd;
	FD_ZERO(&fdlist);
	FD_SET(listenfd, &fdlist);
	for (p = top; p; p = p->next) {
	    FD_SET(p->fd, &fdlist);
	    if (p->fd > maxfd)
		maxfd = p->fd;
	}
	
	if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
	    perror("select");
	} else {
	    for (p = top; p; p = p->next)
		if (FD_ISSET(p->fd, &fdlist)) break;
		/*
		 * it's not very likely that more than one client will drop at
		 * once, so it's not a big loss that we process only one each
		 * select(); we'll get it later...
		 */
	    if (p)
		whatsup(p, user_list);  /* might remove p from list, so can't be in the loop */
	    if (FD_ISSET(listenfd, &fdlist))
		newconnection(user_list);
	}
    }
    
	return 0;
}
   
