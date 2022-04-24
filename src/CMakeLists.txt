cmake_minimum_required(VERSION 3.18)
project(system_call VERSION 1.0)

add_library(lib STATIC defines.c err_exit.c fifo.c semaphore.c shared_memory.c)#static library faster than dynamic library

#set_target_properties(lib PROPERTIES LINKER_LANGUAGE C)