cmake_minimum_required(VERSION 3.18)# cmake minimum version required
project(system_call VERSION 1.0)# set project name and version

set(CMAKE_C_STANDARD 99)# set c standard
#set(CMAKE_C_STANDARD_REQUIRED on)# c standard is require
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Werror -Wpedantic ")# set compilation flags
set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/bin")# aliasing ./bin -> BIN
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})# set output directory -> bin
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)# set building type
#set(FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")# set project license

#include_directories(inc)#compiler reference
add_subdirectory(src)# add src subdir

add_executable(client0 src/client_0.c inc/segnali.h)# create client0 exe
add_executable(server src/server.c)# create server exe

#target_include_directories(lib PUBLIC inc)#compiler reference .h
target_link_libraries(client0 PRIVATE lib)# link lib to client0 exe
target_link_libraries(server PRIVATE lib)# link lib to server exe

#install(TARGETS client0 DESTINATION "${CMAKE_BUILD_TYPE}")# install client0 in /usr/local/bin
#install(TARGETS server DESTINATION "${CMAKE_BUILD_TYPE}")# install server in /usr/local/bin


