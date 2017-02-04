#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    
#include <arpa/inet.h>
#include <string.h>

#define PORT 52792

int main(int argc, char **argv) { 

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
   
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr)< 0) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }
    printf("Client: Client connected\n");
    
    char key_board[128];
    //read from and write to server here.
    
    //get input from keyboard
    printf("Enter a command: ");
    gets(key_board); //should be fgets, but fgets doesn't work
    int len = strlen(key_board);
    printf("len: %d\n", len);
    printf("%c", key_board[len]);
    
    key_board[len+1] = '\r';
    key_board[len+2] = '\n';
    key_board[len+3] = '\0';
    printf("entered: %s\n", key_board);
    write(sock_fd, key_board, len+2); 
    

    close(sock_fd);
    return 0;
}
