#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#define MAX 1024

void sigint_handler(int signal);

int PORT;
const char* ADDR;

int main(int argc, char* argv[]){
	(void)argc;
    int client_fd; // file descriptor of socket
	struct sockaddr_in address; // address structure for IPv4
	char input[MAX]; // stores input from user
    char* response = NULL; // stores response from the server

	PORT = atoi(argv[1]);
	ADDR= argv[2];

	/* signal for terminating server */
	struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = sigint_handler;
    sigemptyset(&(sig.sa_mask));

	sigaction(SIGINT, &sig, NULL);

	/* creating socket */
    client_fd = socket(AF_INET, SOCK_STREAM, 0); // INET y secuencia de caracteres
	if (client_fd < 0) {
		perror("ERROR while creating socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET; // IPv4 protocol
	address.sin_port = htons((uint16_t)PORT); // host-to-network short

	/* convert IPv4 and IPv6 addresses from text to binary form */
	if (inet_pton(AF_INET, ADDR, &address.sin_addr) <= 0) {
		perror("INVALID ADDRESS");
		exit(EXIT_FAILURE);
	}

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
		printf("Enter your journalctl command: ");
		if (fgets(input, MAX, stdin) == NULL){
			perror("fgets");
		}

		/* send message through socket */
		if(write(client_fd, input, sizeof(input)) < 0){
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
        } else {
            /* receive response from server */
            ssize_t bytes_received;
			size_t responseSize = 0;
			char chunk[MAX];

			while ((bytes_received = read(client_fd, chunk, sizeof(chunk))) == MAX) {
				char* newResponse = realloc(response, responseSize + bytes_received + 1);  // +1 for null-terminator
				if (newResponse == NULL) {
					perror("Failed to allocate memory");
					exit(EXIT_FAILURE);
				}
				response = newResponse;
				memcpy(response + responseSize, chunk, bytes_received);
				responseSize += bytes_received;
				// printf("%ld", bytes_received);
			}

			if(bytes_received >=0){
				char* newResponse = realloc(response, responseSize + bytes_received + 1);  // +1 for null-terminator
				if (newResponse == NULL) {
					perror("Failed to allocate memory");
					exit(EXIT_FAILURE);
				}
				response = newResponse;
				memcpy(response + responseSize, chunk, bytes_received);
				responseSize += bytes_received;
			}
			else {
				perror("ERROR WHILE READING RESPONSE");
				exit(EXIT_FAILURE);
			}
				response[responseSize] = '\0';  // Add null-terminator
				printf("Response received. Length: %ld\n", responseSize);
				printf("%s\n", response);
				free(response);
				response = NULL;
			
        }
    }

    return 0;
}

void sigint_handler(int signal){
	printf("\nServer is off, come back later! \n");
	exit(EXIT_SUCCESS);
}