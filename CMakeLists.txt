cmake_minimum_required(VERSION 3.18)# cmake minimum version required # bisogna adattarla a replit 2.8
project(system_call VERSION 1.0)# set project name and version

set(CMAKE_C_STANDARD 99)# set c standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wpedantic ")# set compilation flags
set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/bin")# aliasing ./bin -> BIN
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})# set output directory -> bin
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)# set building type

add_subdirectory(src)# add src subdir

add_executable(client0 src/client_0.c)# create client0 exe
add_executable(server src/server.c)# create server exe

target_link_libraries(client0 PRIVATE lib)# link lib to client0 exe
target_link_libraries(server PRIVATE lib)# link lib to server exe

