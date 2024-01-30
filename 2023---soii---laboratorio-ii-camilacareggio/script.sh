#!/bin/bash

# Number of instances to run
N=1

gnome-terminal --tab --title="SERVER" -- bash -c "cd build/ && ./server 8080 8081 /tmp/unix_socket $N; exec bash"

# Launch N instances of clientA in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="ClientA $i" -- bash -c "cd build/ && ./clientA 8080 127.0.0.1; exec bash"
done

# Launch N instances of clientB in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="ClientB $i" -- bash -c "cd build/ && ./clientB 8081 ::1; exec bash"
done

# Launch N instances of clientC in separate terminal windows
for ((i=1; i<=$N; i++)); do
    gnome-terminal --tab --title="ClientC $i" -- bash -c "cd build/ && ./clientC /tmp/unix_socket; exec bash"
done
