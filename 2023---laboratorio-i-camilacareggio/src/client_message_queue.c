#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#define MAX 100

struct msgbuf {
    long mtype;
    char mtext[MAX];
} message;
  
int main(int argc, char* argv[]) {
    (void)argc;
    key_t key;
    int msgid;
  
    key = ftok(argv[1], 65);
  
    /* get queue id */
    msgid = msgget(key, 0666 | IPC_CREAT);
    message.mtype = 1;

    /* input message from user */
    printf("Ingrese un mensaje: ");
    if (fgets(message.mtext, MAX, stdin) == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);   
    }
  
    /* send message */
    if(msgsnd(msgid, &message, sizeof(message), 0) == -1){
        perror("msgsnd error");
        exit(EXIT_FAILURE);
    }
    return 0;
}