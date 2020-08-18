cmake_minimum_required(VERSION 3.12.4) 

project(dotcvm)

add_compile_definitions(DOTCVM)

file(GLOB_RECURSE SRC_FILES "dotcvm/src/*.cpp")

add_executable(dotcvm ${SRC_FILES})

if(DEBUG)
    if(GDB)
        message("Building with gdb")
        add_compile_definitions(GDB)
    else()
        message("No gdb")
    endif(GDB)
    message("Building in debug mode")
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(DEBUG)
else()
    message("Building in release mode")
    set(CMAKE_BUILD_TYPE Release)
    add_compile_definitions(RELEASE) # Thoug never used
endif(DEBUG)

set_property(TARGET dotcvm PROPERTY CXX_STANDARD 20)

include_directories("dotcvm/include/")

target_link_libraries(dotcvm ${CMAKE_DL_LIBS})
