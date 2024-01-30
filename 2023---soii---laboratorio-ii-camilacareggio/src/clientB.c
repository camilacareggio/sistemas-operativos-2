#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <zlib.h> // Include zlib library for compression
#include <signal.h>
#define MAX 1024

void sigint_handler(int signal);

int PORT;
const char* ADDR;
void receive_compressed_file(int client_fd);
void decompress_file(char* compressed_response, ssize_t bytes_received);

int main(int argc, char* argv[]){
    (void)argc;
    int client_fd; // file descriptor of socket
    struct sockaddr_in6 address; // address structure for IPv6
    char message[MAX]; // stores input from user

    PORT = atoi(argv[1]);
	ADDR= argv[2];

	/* signal for terminating server */
	struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = sigint_handler;
    sigemptyset(&(sig.sa_mask));

	sigaction(SIGINT, &sig, NULL);

    /* creating socket */
    client_fd = socket(AF_INET6, SOCK_STREAM, 0); // IPv6
    if (client_fd < 0) {
        perror("ERROR while creating socket");
        exit(EXIT_FAILURE);
    }

    address.sin6_family = AF_INET6; // IPv6 protocol
    address.sin6_port = htons((uint16_t)PORT); // host-to-network short

    /* convert IPv4 and IPv6 addresses from text to binary form */
    if (inet_pton(AF_INET6, ADDR, &address.sin6_addr) <= 0) {
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
        if (fgets(message, MAX, stdin) == NULL){
            perror("fgets");
        }

        /* send message through socket */
        if(write(client_fd, message, sizeof(message)) < 0){
            perror("ERROR WHILE SENDING MESSAGE");
            exit(EXIT_FAILURE);
        }
        else {
            printf("Message sent\n");
        }

        if(strncmp(message, "exit\0", strlen("exit\0")) == 0){
            printf("Client disconnected\n");

            /* close socket */
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        else {
            receive_compressed_file(client_fd);
        }
    }
    
    return 0;
}

/* recieve compressed data from server:
    reads the compressed response from the server
    prints it and its size (just to check if it is compressed)
    calls decompress_file()*/
void receive_compressed_file(int client_fd){
    char* response = NULL;
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
    // if(bytes_received < 0){
    //     perror("ERROR WHILE RECEIVING RESPONSE");
    //     exit(EXIT_FAILURE);
    // }
    // else{
    //     printf("Received %ld compressed bytes\n", bytes_received);
    //     printf("Compressed file: %s\n", compressed_response);
        // decompress_file(compressed_response, bytes_received);
    // }
}

/* decompress the file received */
// void decompress_file(char* compressed_response, ssize_t bytes_received){
//     char decompressed_response[MAX];
//     uLong decompressed_size = MAX;
//     if (uncompress((Bytef *)decompressed_response, &decompressed_size, (const Bytef *)compressed_response, (uLong)bytes_received) != Z_OK) {
//         perror("ERROR WHILE DECOMPRESSING RESPONSE");
//         exit(EXIT_FAILURE);
//     }
//     else{
//         printf("Decompressed data: %s\n", decompressed_response);
//         strncpy(decompressed_response, "", MAX);
//     }
// }

void sigint_handler(int signal){
	printf("\nServer is off, come back later! \n");
	exit(EXIT_SUCCESS);
}