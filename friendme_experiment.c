#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "friends.h"

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define DELIM " \n"

 #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifndef PORT
  #define PORT 52792
#endif

/* 
 * Print a formatted error message to stderr.
 */
void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr) {
    User *user_list = *user_list_ptr;

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "add_user") == 0 && cmd_argc == 2) {
        switch (create_user(cmd_argv[1], user_list_ptr)) {
            case 1:
                error("user by this name already exists");
                break;
            case 2:
                error("username is too long");
                break;
        }

    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        char * buf = list_users(user_list);
        printf("%s",buf);
        free(buf);
		// modified
    }  else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 3) {
        switch (make_friends(cmd_argv[1], cmd_argv[2], user_list)) {
            case 1:
                error("users are already friends");
                break;
            case 2:
                error("at least one user you entered has the max number of friends");
                break;
            case 3:
                error("you must enter two different users");
                break;
            case 4:
                error("at least one user you entered does not exist");
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 4) {
        // first determine how long a string we need
        int space_needed = 0;
        int i;
        for (i = 3; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[3]);
        for (i = 4; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *author = find_user(cmd_argv[1], user_list);
        User *target = find_user(cmd_argv[2], user_list);
        switch (make_post(author, target, contents)) {
            case 1:
                error("the users are not friends");
                break;
            case 2:
                error("at least one user you entered does not exist");
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        char * user_string;
        if ((user_string = print_user(user)) == NULL) {
            error("user not found");
        }
        printf("%s\n", user_string);
        free(user_string);
    } else {
        error("Incorrect syntax");
    }
    return 0;
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!");
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}


int friend_allocate(char* argv[]) {
    int batch_mode = (argc == 2);
    char input[INPUT_BUFFER_SIZE];
    FILE *input_stream;

    // Create the heads of the empty data structure
    User *user_list = NULL;
	input_stream = stdin;
    printf("Welcome to FriendMe! (Local version)\nPlease type a command:\n> ");
    
    while (fgets(input, INPUT_BUFFER_SIZE, input_stream) != NULL) {
        // only echo the line in batch mode since in interactive mode the user
        // just typed the line
        if (batch_mode) {
            printf("%s", input);
        }

        char *cmd_argv[INPUT_ARG_MAX_NUM];
        int cmd_argc = tokenize(input, cmd_argv);

        if (cmd_argc > 0 && process_args(cmd_argc, cmd_argv, &user_list) == -1) {
            break; // can only reach if quit command was entered
        }

        printf("> ");
    }

    if (batch_mode) {
        fclose(input_stream);
    }

    return 0;
 }

int setup(void) {
  int on = 1, status;
  struct sockaddr_in self;
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // Make sure we can reuse the port immediately after the
  // server terminates.
  status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                      (const char *) &on, sizeof(on));
  if(status == -1) {
    perror("setsockopt -- REUSEADDR");
  }

  self.sin_family = AF_INET;
  self.sin_addr.s_addr = INADDR_ANY;
  self.sin_port = htons(PORT);
  memset(&self.sin_zero, 0, sizeof(self.sin_zero));  // Initialize sin_zero to 0

  printf("Listening on %d\n", PORT);

  if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
    perror("bind"); // probably means port is in use
    exit(1);
  }

  if (listen(listenfd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return listenfd;
}

/*Search the first inbuf characters of buf for a network newline ("\r\n").
  Return the location of the '\r' if the network newline is found,
  or -1 otherwise.
  Definitely do not use strchr or any other string function in here. (Why not?)
*/

int find_network_newline(const char *buf, int inbuf) {
  // Step 1: write this function
  int i;
  for(i=0; i< inbuf;i++){
	  if(buf[i] == '\r'){
		  return i;
	  }
  }
  return -1; // return the location of '\r' if found
}

int main() {
  int listenfd;
  int fd, nbytes;
  char buf[30];
  int inbuf; // how many bytes currently in buffer
  int room; // how much room left in buffer
  char *after; // pointer to position after the (valid) data in buf
  int where; // location of network newline

  struct sockaddr_in peer;
  socklen_t socklen;

  listenfd = setup();
  while (1) {
    socklen = sizeof(peer);
    // Note that we're passing in valid pointers for the second and third
    // arguments to accept here, so we can actually store and use client
    // information.
    if ((fd = accept(listenfd, (struct sockaddr *)&peer, &socklen)) < 0) {
      perror("accept");

    } else {
      printf("New connection on port %d\n", ntohs(peer.sin_port));

      // Receive messages
      inbuf = 0;          // buffer is empty; has no bytes
      room = sizeof(buf); // room == capacity of the whole buffer
      after = buf;        // start writing at beginning of buf

      while ((nbytes = read(fd, after, room)) > 0) {
        // Step 2: update inbuf (how many bytes were just added?)
		inbuf+=nbytes;
        
        // Step 3: call find_network_newline, store result in variable "where"
		where = find_network_newline(buf, inbuf);
        
        if (where >= 0) { // OK. we have a full line

          // Step 4: output the full line, not including the "\r\n",
          // using print statement below.
          // Be sure to put a '\0' in the correct place first;
          // otherwise you'll get junk in the output.
          // (Replace the "\r\n" with appropriate characters so the 
          // message prints correctly to stdout.)
          buf[where] = '\n';
          buf[where+1] = '\0';
          
          printf("Message Read: %s\n", buf);
          // Note that we could have also used write to avoid having to
          // put the '\0' in the buffer. Try using write later!
          
          
          // Step 5: update inbuf and remove the full line from the buffer
          // There might be stuff after the line, so don't just do inbuf = 0
          
          // You want to move the stuff after the full line to the beginning 
          // of the buffer.  A loop can do it, or you can use memmove.
          // memmove(destination, source, number_of_bytes)
          inbuf = inbuf - where;
          memmove( &buf, &(buf[where+2]), inbuf);
        }
        // Step 6: update room and after, in preparation for the next read
		room = room - inbuf;
		after = &(buf[inbuf-1]);     
      }
      close(fd);
    }
  }
  return 0;
}
   

