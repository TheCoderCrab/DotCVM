cmake_minimum_required(VERSION 3.12.4) 

project(dotcvm_standard_user_device)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(dotcvm_standard_user_device SHARED ${SRC_FILES})

if(UNIX)
    find_package(X11 REQUIRED)
    link_directories(${X11_INCLUDE_DIR})
    target_link_libraries(dotcvm_standard_user_device ${X11_LIBRARIES})
endif(UNIX)
if(WIN32)
    message("Windows is not supported for now!")
endif(WIN32)


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


set_property(TARGET dotcvm_standard_user_device PROPERTY CXX_STANDARD 20)

include_directories("../include/")

