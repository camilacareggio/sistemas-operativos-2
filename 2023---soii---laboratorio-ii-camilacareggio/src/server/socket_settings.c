#include "../../headers/socket_settings.h"

int set_ipv4_socket(struct sockaddr_in address_ipv4, int PORTA, int MAX_CONNECTIONS){
	/* creating socket A */
    int socketA_fd = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    if (socketA_fd < 0) {
        perror("ERROR while creating socket B");
        exit(EXIT_FAILURE);
    }
    setsockopt(socketA_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	/* setting socket address for ipv4 */
	address_ipv4.sin_family = AF_INET;
	address_ipv4.sin_addr.s_addr = INADDR_ANY;
	address_ipv4.sin_port = htons((uint16_t)PORTA);

	/* binding address_ipv4 and socket */
	if (bind(socketA_fd, (struct sockaddr*)&address_ipv4, sizeof(address_ipv4)) < 0) {
		perror("BIND FAILED for IPv4");
		exit(EXIT_FAILURE);
	} else {
        printf("IPv4 SERVER ON :)\n");
    }

	/* Prepare to accept connections on socket FD. MAX_CONNECTIONS connection requests will be queued before further requests are refused.*/
	if (listen(socketA_fd, MAX_CONNECTIONS) < 0) {
		perror("LISTEN FAILED");
		exit(EXIT_FAILURE);
	}
	return socketA_fd;
}

int set_ipv6_socket(struct sockaddr_in6 address_ipv6, int PORTB, int MAX_CONNECTIONS){
	/* creating socket B */
    int socketB_fd = socket(AF_INET6, SOCK_STREAM, 0);
    int option = 1;
    if (socketB_fd < 0) {
        perror("ERROR while creating socket B");
        exit(EXIT_FAILURE);
    }
    setsockopt(socketB_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	/* setting socket address for ipv6 */
	address_ipv6.sin6_family = AF_INET6;
	address_ipv6.sin6_addr = in6addr_any;
	address_ipv6.sin6_port = htons((uint16_t)PORTB);

	/* binding address_ipv6 and socket */
	if (bind(socketB_fd, (struct sockaddr*)&address_ipv6, sizeof(address_ipv6)) < 0) {
		perror("BIND FAILED for IPv6");
		exit(EXIT_FAILURE);
	} else {
		printf("IPv6 SERVER ON :)\n");
	}

	/* Prepare to accept connections on socket FD. MAX_CONNECTIONS connection requests will be queued before further requests are refused.*/
	if (listen(socketB_fd, MAX_CONNECTIONS) < 0) {
		perror("LISTEN FAILED");
		exit(EXIT_FAILURE);
	}
	return socketB_fd;
}

int set_unix_socket(struct sockaddr_un address_unix, const char* SOCK_PATH, int MAX_CONNECTIONS) {
    /* creating socket C */
    int socketC_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketC_fd < 0) {
        perror("ERROR while creating socket C");
        exit(EXIT_FAILURE);
    }

    /* setting socket address for Unix domain socket */
    address_unix.sun_family = AF_UNIX;
    strncpy(address_unix.sun_path, SOCK_PATH, sizeof(address_unix.sun_path) - 1);

    /* binding address and socket */
    if (bind(socketC_fd, (struct sockaddr*)&address_unix, sizeof(address_unix)) < 0) {
        perror("BIND FAILED for Unix domain socket");
        exit(EXIT_FAILURE);
    } else {
        printf("Unix domain SERVER ON :)\n");
    }

    /* Prepare to accept connections on socket FD. MAX_CONNECTIONS connection requests will be queued before further requests are refused.*/
    if (listen(socketC_fd, MAX_CONNECTIONS) < 0) {
        perror("LISTEN FAILED");
        exit(EXIT_FAILURE);
    }
    return socketC_fd;
}
