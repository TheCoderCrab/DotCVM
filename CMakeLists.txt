cmake_minimum_required(VERSION 3.12.4) 

project(DotCVM)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(DotCVM SHARED ${SRC_FILES})

if(DEBUG)
message("Building in debug mode")
set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(DEBUG)
else()
message("Building in release mode")
set(CMAKE_BUILD_TYPE Release)
endif()

set_property(TARGET DotCVM PROPERTY CXX_STANDARD 17)
target_include_directories(DotCVM PRIVATE include $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/linux)
