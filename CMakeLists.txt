cmake_minimum_required(VERSION 3.27)
project(Client C)

set(CMAKE_C_STANDARD 17)

add_executable(Client client.c
        server.c
)
