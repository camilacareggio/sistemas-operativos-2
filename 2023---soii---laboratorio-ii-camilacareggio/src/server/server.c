#include "../../headers/server.h"

int SERVER_RUNNING = 1;

int main(int argc, char* argv[]){
	
	(void)argc;

	/* set of file descriptors */
	fd_set rfds;
	int max_fd;

	/* signal for terminating server */
	struct sigaction sig;
    sig.sa_flags = SA_SIGINFO | SA_RESTART;
    sig.sa_handler = sigint_handler;
    sigemptyset(&(sig.sa_mask));

	/* sockets file descriptors and address structures */
	int socketA_fd, socketB_fd, socketC_fd;
	struct sockaddr_in address_ipv4;
	struct sockaddr_in6 address_ipv6;
	struct sockaddr_un address_unix;
	int addrlen_ipv4 = sizeof(address_ipv4); // i need it for the accept function
	int addrlen_ipv6 = sizeof(address_ipv6);
	int addrlen_unix = sizeof(address_unix);

	/* ports and path from arguments */
	int PORTA = atoi(argv[1]);
	int PORTB = atoi(argv[2]);
	const char* SOCK_PATH = argv[3];
	int MAX_CONNECTIONS = atoi(argv[4]);

	/* keeping count of child processes */
	int pids[MAX_CONNECTIONS*3];
	int pid_count=0;

	unlink(SOCK_PATH);

	/* configure sockets */
	socketA_fd = set_ipv4_socket(address_ipv4, PORTA, MAX_CONNECTIONS);
	socketB_fd = set_ipv6_socket(address_ipv6, PORTB, MAX_CONNECTIONS);
	socketC_fd = set_unix_socket(address_unix, SOCK_PATH, MAX_CONNECTIONS);

	sigaction(SIGINT, &sig, NULL);

	/* initializing set of fds */
	FD_ZERO(&rfds);
	FD_SET(socketA_fd, &rfds); // add IPv4 socket to the set

	printf("PID of current process: %d\n", getpid());

	while(get_server_running()){

		/* keeping track of the maximum fd in the set (i'll need it for select function) */
		max_fd = socketA_fd;

		if (socketB_fd > 0) {
			FD_SET(socketB_fd, &rfds); // add IPv6 socket to the set
			max_fd = (socketB_fd > max_fd) ? socketB_fd : max_fd;
		}
		if (socketC_fd > 0) {
			FD_SET(socketC_fd, &rfds); // add Unix socket to the set
			max_fd = (socketC_fd > max_fd) ? socketC_fd : max_fd;
		}

		/* watches out for multiple fds, Unix standard method */
		if (select(max_fd + 1, &rfds, NULL, NULL, NULL) < 0) {
			perror("SELECT FAILED");
			break;
		}

		/* accepts connection if the socket is in the set */
		if (FD_ISSET(socketA_fd, &rfds)) { // IPv4
			int new_socket_ipv4 = accept(socketA_fd, (struct sockaddr*)&address_ipv4, (socklen_t*)&addrlen_ipv4);
			if (new_socket_ipv4 < 0) {
				perror("ACCEPT FAILED (IPv4)");
				exit(EXIT_FAILURE);
			}
			pids[pid_count] = handler_child_process(socketA_fd, new_socket_ipv4, "A");
			pid_count++;
		}
		if (FD_ISSET(socketB_fd, &rfds)) { // IPv6
			int new_socket_ipv6 = accept(socketB_fd, (struct sockaddr*)&address_ipv6, (socklen_t*)&addrlen_ipv6);
			if (new_socket_ipv6 < 0) {
				perror("ACCEPT FAILED (IPv6)");
				exit(EXIT_FAILURE);
			}
			pids[pid_count] = handler_child_process(socketB_fd, new_socket_ipv6, "B");
			pid_count++;
		}
		if (FD_ISSET(socketC_fd, &rfds)) { // Unix
			int new_socket_unix = accept(socketC_fd, (struct sockaddr*)&address_unix, (socklen_t*)&addrlen_unix);
			if (new_socket_unix < 0) {
				perror("ACCEPT FAILED (UNIX)");
				exit(EXIT_FAILURE);
			}
			pids[pid_count] = handler_child_process(socketC_fd, new_socket_unix, "C");
			pid_count++;
		}
	}
	
	for (int i = 0; i < pid_count; i++) {
		if (kill(pids[i], SIGUSR1) >= 0) {
			printf("SIGNAL SENT TO %d\n", pids[i]);
		}

		if (kill(pids[i]-1, SIGINT) >= 0) {
			printf("SIGNAL SENT to the client %d\n", pids[i]-1);
		}
	}

	if(close(socketA_fd) < 0){
		perror("Error while shutting down socketA");
	}
	if(close(socketB_fd) < 0){
		perror("Error while shutting down socketB");
	}
	if(unlink(SOCK_PATH) < 0){
		perror("Error while unlinking socketC");
	}

	printf("SERVER RUNNING: %d\n", get_server_running());
	printf("SERVER OFFLINE\n");
	return 0;
}

/* Creates a child process to handle a Client (A,B,C):
	According to the client type, calls the correspondent client handler function */
int handler_child_process(int socket_fd, int new_socket, char* client_type) {

	char client_pid[6];
	
	pid_t childpid = fork();
	if (childpid == 0) {

		/* close socket in child process */
		close(socket_fd);

		read(new_socket, client_pid, 6);
		int client_pid_int = atoi(client_pid);
		printf("Client %s connected, PID: %d\n", client_type, client_pid_int);

		int client_on=1;
        while(client_on){
			if(strncmp(client_type, "C", strlen("C")) == 0){
				client_on=handle_client_unix(new_socket);
			}
			else{
				client_on=handle_client_ip(new_socket, client_type);
			}
		}
		close(new_socket);
		exit(EXIT_SUCCESS);
	}
	else if (childpid < 0) {
		perror("FORK FAILED");
		exit(EXIT_FAILURE);
	}
	else { // Parent process
		close(new_socket);
	}
	return childpid;
}

int get_server_running(){
	return SERVER_RUNNING;
}

void sigint_handler(int signal) {
		SERVER_RUNNING = 0;
}