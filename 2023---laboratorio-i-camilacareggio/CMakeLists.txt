# Operating Systems II - UNC FCEFyN 2023


# First, we need to specify the CMake version
cmake_minimum_required(VERSION 3.22)

# Then, we setup our project
project(
    App
    VERSION 1.0.0
    DESCRIPTION "ICOMP-UNC/2023---laboratorio-i-camilacareggio"
    LANGUAGES C
)

# Set the C version
set(CMAKE_C_STANDARD 11)

# We create our executable
add_executable(server src/server.c)
add_executable(fifo_client src/client_fifo.c)
add_executable(msg_queue_client src/client_message_queue.c)
add_executable(shmem_client src/client_shared_memory.c)

# The we add the headers of our project
# target_include_directories()

add_compile_options(-Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11)