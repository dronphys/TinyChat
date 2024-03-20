cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(imgui)
add_library(
imgui STATIC
src/imgui.cpp
src/imgui_demo.cpp
src/imgui_draw.cpp
src/imgui_impl_glfw.cpp
src/imgui_impl_opengl3.cpp
src/imgui_tables.cpp
src/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC include/)

target_link_directories(imgui PUBLIC ../glfw)
target_link_libraries(imgui PUBLIC glfw)