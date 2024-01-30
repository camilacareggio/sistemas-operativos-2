# Operating Systems II - UNC FCEFyN 2023


# First, we need to specify the CMake version
cmake_minimum_required(VERSION 3.22)

# Then, we setup our project
project(
    App
    VERSION 1.0.0
    DESCRIPTION "ICOMP-UNC/2023---laboratorio-iii-camilacareggio"
    LANGUAGES C
)

# Set the C version
set(CMAKE_C_STANDARD 11)

# We create our executable
add_executable(mejora-yo mejoras/mejora-yo.c)
add_executable(laboratorio3original laboratorio3original.c)
add_executable(mejora-valgrind mejoras/mejora-valgrind.c)
add_executable(mejora-completo mejoras/mejora-completo.c)
add_executable(test test/test.c test/functions.c)

# The we add the headers of our project
target_include_directories(test PRIVATE "/headers/functions.h")

# add_compile_options(-Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11)
add_compile_options(-O0)
target_link_libraries(test m)