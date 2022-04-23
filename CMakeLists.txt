cmake_minimum_required(VERSION 3.21)
project(system_call VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})
SET(FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include_directories(src inc bin)
add_subdirectory(inc)
add_subdirectory(src)

add_executable(client0 src/client_0.c)
add_executable(server src/server.c)

target_link_libraries(client0 PUBLIC lib clib)
target_link_libraries(server PUBLIC lib clib)
