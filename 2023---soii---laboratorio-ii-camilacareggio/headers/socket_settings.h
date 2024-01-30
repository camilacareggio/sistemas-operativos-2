#ifndef SOCKET_SETTINGS_H
#define SOCKET_SETTINGS_H

#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>
#include <signal.h>

int set_ipv4_socket(struct sockaddr_in address_ipv4, int PORTA, int MAX_CONNECTIONS);
int set_ipv6_socket(struct sockaddr_in6 address_ipv6, int PORTB, int MAX_CONNECTIONS);
int set_unix_socket(struct sockaddr_un address_unix,const char* SOCK_PATH, int MAX_CONNECTIONS);

#endif