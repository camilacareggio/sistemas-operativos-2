#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#define MAX 1024

typedef struct {
    int flag;
    char message[MAX];
} sharedMem;

int main(int argc, char* argv[]) {
    (void)argc;
    int fd;
    sharedMem *data;
    const char *shm_name = argv[1];

    /* open the semaphore */
    sem_t *semaphore = sem_open(argv[2], O_CREAT, 0644, 0);
    if (semaphore == SEM_FAILED) {
        perror("SEMAPHORE OPEN ERROR");
        exit(EXIT_FAILURE);
    }

    /* open shared memory segment */
    fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd < 0) {
        perror("SHM OPEN ERROR");
        exit(EXIT_FAILURE);
    }

    /* map segment */
    data = (sharedMem*) mmap(NULL, sizeof(sharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("MMAP ERROR");
        exit(EXIT_FAILURE);
    }

    printf("Ingrese un mensaje: ");
    if(fgets(data->message, sizeof(data->message), stdin) == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);  
    }

    data->flag = 1;
    if(sem_post(semaphore) < 0){ // unlock
        perror("semaphore cant be unlocked");
        exit(EXIT_FAILURE);  
    }

    printf("Message sent to server!\n");

    /* dealloc mmap */
    if(munmap(data, sizeof(sharedMem)) < 0){
        perror("UNMAP ERROR");
            exit(EXIT_FAILURE);
    }

    /* close fd */
    if(close(fd) < 0){
        perror("ERROR while closing fd");
        exit(EXIT_FAILURE);  
    }

    return 0;
}