cmake_minimum_required(VERSION 3.15) 

project(DotCVM)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")


add_library(DotCVM SHARED ${SRC_FILES})
set_property(TARGET DotCVM PROPERTY CXX_STANDARD 17)
target_include_directories(DotCVM PRIVATE include $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/linux)
