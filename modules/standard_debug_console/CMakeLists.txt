cmake_minimum_required(VERSION 3.12.4) 

project(dotcvm_standard_debug_console)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(dotcvm_standard_debug_console SHARED ${SRC_FILES})

if(DEBUG)
    if(GDB)
        add_compile_definitions(GDB)
    endif(GDB)
    message("Building in debug mode")
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(DEBUG)
else()
    message("Building in release mode")
    set(CMAKE_BUILD_TYPE Release)
    add_compile_definitions(RELEASE) # Thoug never used
endif(DEBUG)


set_property(TARGET dotcvm_standard_debug_console PROPERTY CXX_STANDARD 20)

include_directories("../include/")

