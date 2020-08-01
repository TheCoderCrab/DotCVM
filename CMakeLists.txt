cmake_minimum_required(VERSION 3.12.4) 

project(dotcvm)

file(GLOB_RECURSE SRC_FILES "dotcvm/src/*.cpp")

add_executable(dotcvm ${SRC_FILES})

if(DEBUG)
message("Building in debug mode")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --save-temps")
add_compile_definitions(DEBUG)
else()
message("Building in release mode")
set(CMAKE_BUILD_TYPE Release)
add_compile_definitions(RELEASE) # Thoug never used
endif(DEBUG)

set_property(TARGET dotcvm PROPERTY CXX_STANDARD 20)

include_directories("dotcvm/include/")

message(${CMAKE_DL_LIBS})
target_link_libraries(dotcvm ${CMAKE_DL_LIBS})
