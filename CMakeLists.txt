cmake_minimum_required(VERSION 3.20)

project(Pokemoon)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_subdirectory(third-party/glfw)

add_executable(main main.cpp logging.cpp platform.cpp)

target_link_libraries(main PRIVATE Vulkan::Vulkan glfw)

target_compile_options(main PRIVATE "$<$<CONFIG:DEBUG>:-D DEBUG>")
