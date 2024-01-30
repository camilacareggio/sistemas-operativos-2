#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "../cJSON/cJSON.h"
#include "server.h"

int handle_client_ip(int new_socket, char* client_type);
int handle_client_unix(int new_socket_unix);
char* get_mem_free();
char* get_load_avg();
void send_compressed_file(char* json_str, int new_socket);

#endif
