cmake_minimum_required(VERSION 3.21)
project(Clion)
include_directories(src inc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_C_STANDARD 11)

add_executable(client0 src/client/client_0.c src/defines.c)
add_executable(server src/server/server.c)


