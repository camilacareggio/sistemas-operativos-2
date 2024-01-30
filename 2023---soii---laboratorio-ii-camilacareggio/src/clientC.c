#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#define MAX 1024

void sigint_handler(int signal);

const char* SOCK_PATH;

int main(int argc, char* argv[]){
    (void)argc;
    int client_fd; // file descriptor of socket
    struct sockaddr_un address; // address structure for Unix socket
    char input[MAX]; // stores input from user
    char response[MAX]; // stores response from the server

    SOCK_PATH = argv[1];

	/* signal for terminating server */
	struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = sigint_handler;
    sigemptyset(&(sig.sa_mask));

	sigaction(SIGINT, &sig, NULL);

    /* creating socket */
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0); // Unix socket
    if (client_fd < 0) {
        perror("ERROR while creating socket");
        exit(EXIT_FAILURE);
    }

    address.sun_family = AF_UNIX; // Unix protocol
    strncpy(address.sun_path, SOCK_PATH, sizeof(address.sun_path) - 1);

    /* ask for a connection */
    if (connect(client_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("CANNOT CONNECT");
        exit(EXIT_FAILURE);
    }

    pid_t cl_pid = getpid();
	char cl_pid_str[6];
	snprintf(cl_pid_str, sizeof(cl_pid_str), "%d", cl_pid);

	if(write(client_fd, cl_pid_str, sizeof(cl_pid_str)) < 0){
		perror("ERROR WHILE SENDING MESSAGE");
        exit(EXIT_FAILURE);
	}
	else {
		printf("PID %d sent\n", cl_pid);
	}
    while(1){
        /* user message input */
        printf("Enter your command: ");
        if (fgets(input, MAX, stdin) == NULL){
            perror("fgets");
        }

        /* send message through socket */
        if(write(client_fd, input, strlen(input)) < 0){
            perror("ERROR WHILE SENDING MESSAGE");
            exit(EXIT_FAILURE);
        }
        else {
            printf("Message sent\n");
        }

        if(strncmp(input, "exit\0", strlen("exit\0")) == 0){
            printf("Client disconnected\n");
            /* close socket */
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        else{
            /* receive response from server */
            ssize_t bytes_received = read(client_fd, response, MAX);
            if(bytes_received < 0){
                perror("ERROR WHILE READING RESPONSE");
                exit(EXIT_FAILURE);
            }
            else{
                printf("Response recieved. Lenght: %ld\n", bytes_received);
                printf("%s\n", response);
                strncpy(response, "", MAX);        
            }
        }
    }

    

    return 0;
}

void sigint_handler(int signal){
	printf("\nServer is off, come back later! \n");
	exit(EXIT_SUCCESS);
}
