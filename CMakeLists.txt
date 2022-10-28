cmake_minimum_required(VERSION 3.20)

project(Pokemoon)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/glm)

set(SOURCES
    main.cpp
    logging.cpp
    platform.cpp
    application.cpp
    memory.cpp
    event.cpp
    container/darray.cpp
    input.cpp
    StringUtils.cpp
    core/Clock.cpp
    renderer/frontend.cpp
    renderer/backend.cpp
    renderer/device.cpp
    renderer/swapchain.cpp
    renderer/image.cpp
    renderer/render_pass.cpp
    renderer/command_buffer.cpp
    renderer/framebuffer.cpp
    renderer/fence.cpp
    renderer/shaders/ObjectShader.cpp
    renderer/Pipeline.cpp
    renderer/buffer.cpp
    memory/LinearAllocator.cpp
    platform/filesystem.cpp)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(main PRIVATE Vulkan::Vulkan glfw glm)

target_compile_options(main PRIVATE "$<$<CONFIG:DEBUG>:-D DEBUG>")
