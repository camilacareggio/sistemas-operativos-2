#!/bin/bash

# Number of instances to run
N=3


gnome-terminal --tab --title="SERVER" -- bash -c "cd build/ && ./server /tmp/client_to_server_fifo /tmp/server_to_client_fifo progfile /my_shm /my_semaphore; exec bash"

# Launch N instances of clientA in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="Client FIFO $i" -- bash -c "cd build/ && ./fifo_client /tmp/client_to_server_fifo /tmp/server_to_client_fifo; exec bash"
done

# Launch N instances of clientB in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="Client Msg Queue $i" -- bash -c "cd build/ && ./msg_queue_client progfile; exec bash"
done

# Launch N instances of clientC in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="Client Shared memory $i" -- bash -c "cd build/ && ./shmem_client /my_shm /my_semaphore; exec bash"
done
