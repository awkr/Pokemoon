cmake_minimum_required(VERSION 3.20)

project(Pokemoon)

set(CMAKE_CXX_STANDARD 23)

add_executable(main main.cpp)

target_compile_options(main PRIVATE "$<$<CONFIG:DEBUG>:-D DEBUG>")
