cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
set(PROJECT_NAME TinyChat)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_SERVER "Build the server" ON)
option(BUILD_CLIENT "Build the client" ON)


add_library(asio INTERFACE)
include_directories(asio INTERFACE include/asio/include)

add_library(networking INTERFACE)
include_directories(networking INTERFACE include/net)
#linging my net lib and asio
target_link_libraries(networking INTERFACE asio)

if (BUILD_CLIENT)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    add_subdirectory(include/glfw)
    add_subdirectory(include/glad)
    add_subdirectory(include/imgui)

    add_subdirectory(src/client)
endif()

if (BUILD_SERVER)
    add_subdirectory(src/server)
endif()

