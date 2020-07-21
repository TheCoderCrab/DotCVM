cmake_minimum_required(VERSION 3.12.4) 

project(DotCVM)

find_package(X11 REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(DotCVM ${SRC_FILES})

if(DEBUG)
message("Building in debug mode")
set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(DEBUG)
else()
message("Building in release mode")
set(CMAKE_BUILD_TYPE Release)
add_compile_definitions(RELEASE)
endif()

set_property(TARGET DotCVM PROPERTY CXX_STANDARD 17)

include_directories(${X11_INCLUDE_DIR} "include/")

link_directories(${X11_LIBRARIES})

target_link_libraries(DotCVM ${X11_LIBRARIES})
