cmake_minimum_required(VERSION 3.21)
project(system_call VERSION 1.0)

file(GLOB LIB_SRC
        "*.h"
        )

add_library(lib ${LIB_SRC})

set_target_properties(lib PROPERTIES LINKER_LANGUAGE C)