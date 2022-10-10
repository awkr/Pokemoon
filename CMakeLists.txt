cmake_minimum_required(VERSION 3.20)

project(Pokemoon)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_subdirectory(third-party/glfw)

set(SOURCES
    main.cpp
    logging.cpp
    platform.cpp
    application.cpp
    memory.cpp
    event.cpp
    container/darray.cpp
    input.cpp
    core/clock.cpp
    renderer/frontend.cpp
    renderer/backend.cpp)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(main PRIVATE Vulkan::Vulkan glfw)

target_compile_options(main PRIVATE "$<$<CONFIG:DEBUG>:-D DEBUG>")
