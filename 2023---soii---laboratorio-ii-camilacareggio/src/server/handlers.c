#include "../../headers/handlers.h"

/* handles IPv4 and IPv6 clients (A,B): 
	recieves command from the client
	if it's a journalctl command, it's executed in a pipe 
	else, it notifies invalid command
	if it's Client A, send a json with the command and output
	if it's Client B, go to send_compressed_file() */
int handle_client_ip(int new_socket, char* client_type){

	/* recieving message from client */
	char message[MAX];

	/* JSON object with the response */
	cJSON *response = cJSON_CreateObject();

	if(read(new_socket, message, MAX) < 0){
		cJSON_AddStringToObject(response, "output", "Sorry, there was an error while reading");
	}
	else{
		
		cJSON_AddStringToObject(response, "journalctl_cmd", message);
        /* verify journalctl command */
		if (strncmp(message, "journalctl", strlen("journalctl")) == 0) {

			printf("Received command from Client %s: %s", client_type, message);

			/* open a pipe to execute the journalctl command */ 
			FILE* fp = popen(message, "r");

			if (fp == NULL) {
				perror("CANNOT OPEN PIPE");
				exit(EXIT_FAILURE);
			}

			/* read the output of the command */
			char* output = NULL;
			size_t outputSize = 0;
			size_t outputLength = 0;
			char line[MAX];

			while (fgets(line, sizeof(line), fp) != NULL) {
				size_t lineLength = strlen(line);
				size_t newLength = outputLength + lineLength;

				if (newLength >= outputSize) {
					outputSize = newLength + 1;  // Add 1 for null-terminator
					char* newOutput = realloc(output, outputSize);
					if (newOutput == NULL) {
						perror("Failed to allocate memory");
						exit(EXIT_FAILURE);
					}
					output = newOutput;
				}

				memcpy(output + outputLength, line, lineLength);
				outputLength += lineLength;
			}

			output[outputLength] = '\0';  // Add null-terminator

			cJSON_AddStringToObject(response, "output", output);

			/* Close the pipe */
			pclose(fp);
		}
		else if(strncmp(message, "exit\0", strlen("exit\0")) == 0){
			printf("Client disconnected\n");
			cJSON_Delete(response);
			return 0;
		}
		else {
			printf("Invalid command: %s", message);
			cJSON_AddStringToObject(response, "output", "Invalid command");
		}
	}

	/* JSON object to string so i can send it */
    char *json_str = cJSON_Print(response);
	cJSON_Delete(response);

	/* if Client B, then the call send_compressed_file(), else send the json string */
	if(strcmp(client_type, "B") == 0){
		send_compressed_file(json_str, new_socket);
	}
	else{

		if (write(new_socket, json_str, strlen(json_str)) < 0) {
			perror("ERROR WHILE SENDING MESSAGE");
			exit(EXIT_FAILURE);
		} else {
			printf("Response sent to Client A. Size: %ld bytes\n", strlen(json_str));
		}

	}

    free(json_str);
	return 1;
}

/* handle Unix Client C:
	receives a message from the client
	gets the Free memory available and the Load Average
	sends a JSON with that info back to the client */
int handle_client_unix(int new_socket_unix){

	/* recieving message from client */
	char message[MAX];

	/* JSON object with the report */
    cJSON *root = cJSON_CreateObject();

	if(read(new_socket_unix, message, MAX) < 0){
		cJSON_AddStringToObject(root, "output", "Sorry, there was an error while reading");
	}
	else{
		if(strncmp(message, "exit\0", strlen("exit\0")) == 0){
			printf("Client disconnected\n");
			cJSON_Delete(root);
			return 0;
		}
		printf("Recieved message from Client C: %s", message);
		cJSON_AddStringToObject(root, "mem_free", get_mem_free());
		cJSON_AddStringToObject(root, "load_avg", get_load_avg());
	}

	/* JSON object to string so i can send it */
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);

	if(write(new_socket_unix, json_str, strlen(json_str)) < 0){
		perror("ERROR WHILE SENDING MESSAGE");
		exit(EXIT_FAILURE);
	}
	else {
		printf("Response sent to Client C. Size: %ld bytes\n", strlen(json_str));
	}

    free(json_str);
	return 1;
}

/* Sends compressed file (for Client B IPv6):
	compresses the json string,
	writes it in a compressed file,
	reads the compressed data from the file,
	sends it back to Client B */
void send_compressed_file(char* json_str, int new_socket) {
    // Compress the JSON string
    uLongf compressed_size = compressBound(strlen(json_str));
    Bytef* compressed_str = (Bytef*)malloc(compressed_size);
    if (compressed_str == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    if (compress(compressed_str, &compressed_size, (const Bytef*)json_str, strlen(json_str)) != Z_OK) {
        perror("ERROR WHILE COMPRESSING MESSAGE");
        free(compressed_str);
        exit(EXIT_FAILURE);
    }

    // Write the compressed data to the socket
    if (write(new_socket, compressed_str, compressed_size) < 0) {
        perror("ERROR WHILE SENDING MESSAGE");
        free(compressed_str);
        exit(EXIT_FAILURE);
    } else {
        printf("Response sent to Client B. Size: %ld bytes\n", compressed_size);
    }

    free(compressed_str);
}

/* Gets the free memory: 
	opens a pipe and executes cat /proc/meminfo,
	stores the MemFree value obtained in the output,
	closes pipe and
	returns memfree value */
char* get_mem_free(){

	FILE* fp = popen("cat /proc/meminfo", "r");

    if (fp == NULL) {
        perror("CANNOT OPEN PIPE");
        exit(EXIT_FAILURE);
    }

	char output[MAX];
	char *value;
    /* read the MemFree */
	while (fgets(output, sizeof(output), fp) != NULL) {
		if(strncmp(fgets(output, sizeof(output), fp), "MemFree:", strlen("MemFree:")) == 0){
			/* get MemFree value */
			value = strtok(output, " ");
            value = strtok(NULL, " ");
			break;
		}
	}
    /* closing pipe */
    pclose(fp);
	return value;
}

/* Gets the load average: 
	opens a pipe and executes cat /proc/loadavg,
	stores the first value obtained in the output,
	closes pipe and
	returns loadavg value */
char* get_load_avg(){

	FILE* fp = popen("cat /proc/loadavg", "r");
    
    if (fp == NULL) {
        perror("CANNOT OPEN PIPE");
        exit(EXIT_FAILURE);
    }

	char output[MAX];
	char *value;
    /* read the first value of load avg */
	if (fgets(output, sizeof(output), fp) != NULL) {
		value = strtok(output, " ");
	}

    /* closing pipe */
    pclose(fp);
	return value;
}