cmake_minimum_required(VERSION 3.26)
project(wine_font_reproduce)

set(CMAKE_CXX_STANDARD 17)

add_executable(wine_font_reproduce main.cpp)
target_compile_options(wine_font_reproduce PRIVATE -Wl,-subsystem,windows -municode)
target_link_options(wine_font_reproduce PRIVATE -static) # Only for MinGW
target_link_libraries(wine_font_reproduce PRIVATE d3d9)
