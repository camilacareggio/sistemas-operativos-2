# Laboratorio 1 - IPC
## FIFO
En primer lugar en el servidor se crean las fifo con `mkfifo()` especificando el path y el modo, usé 0666 que permite la escritura y lectura a cualquier proceso. Luego se abren las FIFO, solo con permiso de escritura para la FIFO que viene del cliente y solo con permiso de escritura para la FIFO que va hacia el cliente.  
  
En el cliente, se espera el input del usuario, que será el mensaje a enviar. Una vez que se ingresa,  abrimos las FIFOs y se escribe el mensaje con `write(fifo, mensaje, size-mensaje)`. Chequeamos que no haya un error al escribir, si lo hay, se imprime y se finaliza la ejecución del cliente.  
   
Luego, el servidor que está leyendo la FIFO con `read(fifo, mensaje, size-mensaje)` (se chequea que no haya error de lectura) recibe el mensaje y lo imprime. También le envía un mensaje estándar "Thank you for your message!" al cliente a través de la fifo de servidor a cliente.  
  
El cliente lee el mensaje de la fifo de servidor a cliente y lo imprime. Por último cierra las fifo.  
  
El servidor limpia el contenido del mensaje recibido **s** y queda esperando un nuevo mensaje. 

## Message Queue
 #### 1. Struct del mensaje 
```
struct msgbuf {
    long mtype;
    char mtext[MAX];
} message;
```
donde
- `mtype`: número que etiqueta cada mensaje en la cola
- `mtext`: string
 #### 2. Creación y obtención de ID
```
key = ftok("progfile", 65);
msgid = msgget(key, 0666 | IPC_CREAT);
```
donde
- `key`: identificador del objeto SVIPC
- `msgflg`: Define el comportamiento dependiendo si la cola ya existe o no y tambien la atribución de permisos de acceso a la cola de forma similar al acceso de archivos
- `msgid`: id de la queue
 #### 3. Escribir en la cola
```
msgsnd(msgid, &message, sizeof(message), 0)
```
donde indicamos el id, el mensaje a enviar, su tamaño y la flag
 #### 4. Leer de la cola (en el server)
 ```
 msgrcv(msgid, &message, sizeof(message), 1, 0)
 ```
 donde indicamos el id, la struct, el tamaño máximo y la flag
  #### 5. Eliminar la cola
  ```
  msgctl(msgid, IPC_RMID, NULL);
  ```
Me guié con el ejemplo de este link: https://www.geeksforgeeks.org/ipc-using-message-queues/ 
## Shared Memory
Segmento de memoria en el que los procesos pueden leer y escribir.
#### 1. Creación del segmento
```
key = ftok("shmfile", 60);
shmid = shmget(key, MAX, 0666 | IPC_CREAT);
```
donde
- `key`: identificador
- `MAX`: tamaño del segmento de memoria reservado
- `IPC_CREATE`: se crea el segmento de memoria si no existía sino la flag 0666
- `shmid`: se retorna la ID del segmento
#### 2. Conexión al segmento
```
str = (char*) shmat(shmid, (void*)0, 0);
```
donde
- `shmid`: id del segmento
- `shmaddr`: indica qué area de memoria se pretende. Esto debería dejarse como decisión al OS, indicando un valor: `(void *)0`
#### 3. Mensaje
En el cliente, el usuario ingresa el mensaje a enviar en el segmento de memoria compartida.  
El servidor lee el mensaje que se encuentra en el segmento de memoria y lo imprime. Luego vacía el segmento copiando un string vacío: `strncpy(str, "", MAX);`
#### 4. Desconexión y destrucción del segmento
```
/* detach segment */
shmdt(str);
/* destroy segment */
shmctl(shmid,IPC_RMID,NULL);
```
donde 
- `IPC_RMID`: constante que indica eliminación