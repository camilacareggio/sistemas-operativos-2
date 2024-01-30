#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#define MAX 1024

struct msgbuf {
   long mtype;
   char mtext[100];
} message;

typedef struct {
   int flag;
   char message[MAX];
} sharedMem;

int read_from_fifo(char* fifo1, char* fifo2, char* s, char* s2, FILE* file);
int read_from_message_queue(FILE* file, char* path);
int read_from_shared_memory(char* semaphore_path, char* shm_path, FILE* file);

int main(int argc, char* argv[]) {
    (void)argc;

    /* Create file to store messages */
    FILE* file;
    file = fopen("messages.txt", "w");
    if (file == NULL) {
        perror("Error while opening file.");
    }
    fclose(file);

    /* FIFO */
    char s[BUFSIZ];
    char s2[] = "Recieved!";

    char* fifo1 = argv[1];
    char* fifo2 = argv[2];

    /* create FIFO */
    mkfifo(fifo1, 0666);
    mkfifo(fifo2, 0666);

    /* open file for appending */
    file = fopen("messages.txt", "a");
    if (file == NULL) {
        perror("Error while opening file.");
    }

    /* recieve FIFO message */
    int fifo = read_from_fifo(fifo1, fifo2, s, s2, file);
    if(fifo == -1){
        printf("ERROR FIFO");
    }

    /* recieve Message Queue */
    char* path_msgq = argv[3];
    int msg_queue = read_from_message_queue(file, path_msgq);
    if(msg_queue == -1){
        printf("ERROR MESSAGE QUEUE");
    }
    
    /* recieve Shared memory messages */
    char* shm_path = argv[4];
    char* sempahore_path = argv[5];
    int shm = read_from_shared_memory(sempahore_path, shm_path, file);
    if(shm == -1){
        printf("ERROR SHARED MEMORY");
    }

    printf("SERVER ON\n");
        
    /* close file */
    fclose(file);

    waitpid(msg_queue, NULL, WUNTRACED);
    waitpid(fifo, NULL, WUNTRACED);
    waitpid(shm, NULL, WUNTRACED);

    return 0;
}

int read_from_fifo(char* fifo1, char* fifo2, char* s, char* s2, FILE* file){
    int pid = fork();
    if(pid==0){
        int client_to_server = open(fifo1, O_RDONLY); // only reading permission
        int server_to_client = open(fifo2, O_WRONLY); // only writing permission

        if(client_to_server == -1){
            perror("Error al abrir FIFO");
            exit(EXIT_FAILURE);
        }
        else {
            while(1){
                /* read from fifo */
                if(read(client_to_server, s, BUFSIZ) == -1){
                    perror("Read fifo error");
                    exit(EXIT_FAILURE);
                }
                else{
                    /* print message */
                    if (strcmp(s,"") != 0) {
                        printf("FIFO Client: %s", s);
                        /* write message into file */
                        fputs("FIFO Client: ", file);
                        fputs(s, file);
                        fflush(file);
                        if(server_to_client == -1){
                            perror("Error al abrir FIFO");
                            exit(EXIT_FAILURE);
                        } 
                        else{
                            /* send a response to the client */
                            if(write(server_to_client, s2, sizeof(s2)) == -1){
                                perror("Write fifo error");
                                exit(EXIT_FAILURE);
                            }
                            strcpy(s, "");
                        }  
                    }
                }
            }
        }

        /* close fifos */
        close(client_to_server);
        close(server_to_client);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0){
        return pid;
    }
    else {
        return -1;
    }
}

int read_from_message_queue(FILE* file, char* path){
    int pid = fork();
    if(pid == 0) {
        /* get key */
        key_t key = ftok(path, 65);

        /* create message queue */
        int msgid = msgget(key, 0666 | IPC_CREAT | IPC_NOWAIT);
        while(1){
            
            /* recieve Message Queue message */
            if(msgrcv(msgid, &message, sizeof(message), 1, 0) == -1){
                perror("msgrcv error");
                exit(EXIT_FAILURE);
            }
            printf("Message Queue Client: %s", message.mtext);
            /* write message into file */
            fputs("Message Queue Client: ", file);
            fputs(message.mtext, file);
            fflush(file);
        }
        /* remove message queue */
        msgctl(msgid, IPC_RMID, NULL);
        unlink(path);
        exit(EXIT_SUCCESS);
    }
    else if(pid > 0) {
        return pid;
    }
    else {
        return -1;
    }
}

int read_from_shared_memory(char* semaphore_path, char* shm_path, FILE* file){
    int pid = fork();
    if(pid == 0){
        sharedMem *data;

        /* open shared memory segment */
        int fd = shm_open(shm_path, O_CREAT|O_RDWR, 0666);

        /* set length of fd */
        ftruncate(fd, sizeof(sharedMem));

        /* map segment */
        data = (sharedMem*) mmap(NULL, sizeof(sharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        if (data == MAP_FAILED) {
            perror("MMAP ERROR");
            exit(EXIT_FAILURE);
        }

        data->flag = 0;

        /* open semaphore */
        sem_t *semaphore = sem_open(semaphore_path, O_CREAT, 0644, 0);
        if (semaphore == SEM_FAILED) {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
        // printf("Semaphore created successfully\n");

        while(1){
            /* wait until client has finished writing */
            sem_wait(semaphore);
            // printf("Semaphore acquired by server\n");
            if(data->flag){
                printf("Message Shared Memory Client: %s",data->message);
                /* write message into file */
                fputs("Message Shared Memory Client: ", file);
                fputs(data->message, file);
                fflush(file);
                data->flag = 0;
            }
        }

        munmap(data, sizeof(sharedMem));
        close(fd);

        exit(EXIT_SUCCESS);
    }
    else if(pid > 0) {
        return pid;
    }
    else {
        return -1;
    }
}
