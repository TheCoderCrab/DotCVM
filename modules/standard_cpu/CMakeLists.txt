cmake_minimum_required(VERSION 3.12.4) 

project(dotcvm_standard_cpu)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

find_package(Threads REQUIRED)

add_library(dotcvm_standard_cpu SHARED ${SRC_FILES})

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


set_property(TARGET dotcvm_standard_cpu PROPERTY CXX_STANDARD 20)

include_directories("../include/")

target_link_libraries(dotcvm_standard_cpu ${CMAKE_THREAD_LIBS_INIT})
