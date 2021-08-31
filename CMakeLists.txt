cmake_minimum_required(VERSION 3.9)
project(game-proj)

set(CMAKE_CXX_STANDARD 17)


find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(game-proj)
target_sources(game-proj PRIVATE main.cpp application.hpp glfw_window.hpp severity.hpp version.hpp vulkan.hpp)
target_compile_definitions(game-proj PUBLIC GLFW_INCLUDE_VULKAN)
target_link_libraries(game-proj glfw)
target_link_libraries(game-proj glm::glm)
target_link_libraries(game-proj Vulkan::Vulkan)