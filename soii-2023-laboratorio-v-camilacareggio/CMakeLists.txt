# Operating Systems II - UNC FCEFyN 2023


# First, we need to specify the CMake version
cmake_minimum_required(VERSION 3.22)

# Then, we setup our project
project(
    App
    VERSION 1.0.0
    DESCRIPTION "ICOMP-UNC/2023---laboratorio-v-camilacareggio"
    LANGUAGES C
)

# Set the C version
set(CMAKE_C_STANDARD 11)

# We create our executable
add_executable(api_server api_server.c)
add_executable(client client.c)

# The we add the headers of our project
# target_include_directories(server PRIVATE "cJSON/cJSON.h headers/server.h")

add_compile_options(-Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11)
target_link_libraries(api_server ulfius jansson)
target_link_libraries(client curl)