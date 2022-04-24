cmake_minimum_required(VERSION 3.18)
project(system_call VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Werror -Wpedantic")
set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})
#set(FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

#include_directories(inc)#compiler reference
add_subdirectory(src)

add_executable(client0 src/client_0.c)
add_executable(server src/server.c)

target_include_directories(lib PUBLIC inc)#compiler reference .h
target_link_libraries(client0 PRIVATE lib)#linker reference .c
target_link_libraries(server PRIVATE lib)


