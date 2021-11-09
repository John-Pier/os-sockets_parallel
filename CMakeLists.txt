cmake_minimum_required(VERSION 3.20)

project(lab_client C)
set(CMAKE_C_STANDARD 11)
add_executable(lab_client client/client.c)

project(lab_server C)
set(CMAKE_C_STANDARD 11)
add_executable(lab_server server/server.c)
