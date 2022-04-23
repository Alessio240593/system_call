cmake_minimum_required(VERSION 3.21)
project(system_call VERSION 1.0)

add_library(clib defines.c err_exit.c fifo.c semaphore.c shared_memory.c)

set_target_properties(clib PROPERTIES LINKER_LANGUAGE C)