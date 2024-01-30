#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    (void)argc;
    char s[BUFSIZ];

    int client_to_server;
    char* fifo1 = argv[1];

    int server_to_client;
    char* fifo2 = argv[2];

    /* input message from user */
    printf("Ingrese un mensaje: ");
    if (fgets(s, 199, stdin) == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);   
    }

    /* open fifos */
    client_to_server = open(fifo1, O_WRONLY); // only writing permission
    if(client_to_server < 0){
        perror("ERROR while opening client_to_server");
        exit(EXIT_FAILURE);
    }
    server_to_client = open(fifo2, O_RDONLY); // only reading permission
    if(server_to_client < 0){
        perror("ERROR while opening server_to_client");
        exit(EXIT_FAILURE);
    }
    
    /* write message in FIFO */
    if(write(client_to_server, s, sizeof(s)) == -1){
        perror("Write error");
        exit(EXIT_FAILURE);
    };

    /* read recieved message */
    if(read(server_to_client, s, sizeof(s)) == -1){
        perror("Read error");
        exit(EXIT_FAILURE);
    }
    printf("Received from server: %s",s);

    /* close fds */
    if(close(client_to_server) < 0){
        perror("Closing error");
        exit(EXIT_FAILURE);
    }
    if(close(server_to_client) < 0){
        perror("Closing error");
        exit(EXIT_FAILURE);
    }

    return 0;
}