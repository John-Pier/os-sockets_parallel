cmake_minimum_required(VERSION 3.20)

project(lab_client C)
set(CMAKE_C_STANDARD 11)
add_executable(lab_client client/client.c)

project(lab_server C)
set(CMAKE_C_STANDARD 11)
add_executable(lab_server server/server.c)

project(lab_state-variable C)
set(CMAKE_C_STANDARD 11)
add_executable(lab_state-variable state-variable/state-variable.c)

project(lab_2 C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)
add_executable(lab_2 main.c)
