#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <zlib.h>
#include <signal.h>
#include "../cJSON/cJSON.h"
#include "socket_settings.h"
#include "handlers.h"

#define MAX 1024

int handler_child_process(int socket_fd, int new_socket, char* client_type);
void sigint_handler(int signal);
int get_server_running();

#endif