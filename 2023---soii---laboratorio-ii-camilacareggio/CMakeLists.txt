# Operating Systems II - UNC FCEFyN 2023


# First, we need to specify the CMake version
cmake_minimum_required(VERSION 3.22)

# Then, we setup our project
project(
    App
    VERSION 1.0.0
    DESCRIPTION "ICOMP-UNC/2023---laboratorio-ii-camilacareggio"
    LANGUAGES C
)

# Set the C version
set(CMAKE_C_STANDARD 11)

# We create our executable
add_executable(server src/server/server.c src/server/socket_settings.c src/server/handlers.c cJSON/cJSON.c)
add_executable(clientA src/clientA.c)
add_executable(clientB src/clientB.c)
add_executable(clientC src/clientC.c)

# The we add the headers of our project
target_include_directories(server PRIVATE "cJSON/cJSON.h headers/server.h")

add_compile_options(-Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11)
target_link_libraries(server z)
target_link_libraries(clientB z)