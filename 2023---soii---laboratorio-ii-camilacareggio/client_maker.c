#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
   int n = atoi(argv[1]); // number of intances of each client
    
    for (int i = 0; i < n; i++) {
        pid_t pidA = fork();
        if (pidA == 0) {
            execl("./build/clientA", "clientA", "8080", "127.0.0.1", (char*) NULL);
        } else if (pidA < 0) {
            printf("Error creating process for clientA\n");
            exit(EXIT_FAILURE);
        }

        pid_t pidB = fork();
        if (pidB == 0) { 
            execl("./build/clientB", "clientB", "8081", "::1", (char*) NULL);
        } else if (pidB < 0) {
            printf("Error creating process for clientB\n");
            exit(EXIT_FAILURE);
        }

        pid_t pidC = fork();
        if (pidC == 0) { 
            execl("./build/clientC", "clientC", "/tmp/unix_socket", (char*) NULL);
        } else if (pidC < 0) {
            printf("Error creating process for clientC\n");
            exit(EXIT_FAILURE);
        }
    }

    /* wait until child processes finished */
    for (int i = 0; i < 3*n; i++) {
        wait(NULL);
    }
    
    return 0;
}
