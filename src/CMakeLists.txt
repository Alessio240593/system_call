cmake_minimum_required(VERSION 3.18)# state cmake minimum version # bisogna adattarla a replit 2.8
project(lib VERSION 1.0)# set project name and version
set(INC "${CMAKE_SOURCE_DIR}/inc")# aliasing INC -> ./inc

add_library(${PROJECT_NAME} STATIC defines.c err_exit.c fifo.c semaphore.c shared_memory.c _signal.c message_queue.c)# create lib.a (static)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC})# include header file *.h