cmake_minimum_required(VERSION 3.18)
project(game-proj)

# Set C++ standard used
set(CMAKE_CXX_STANDARD 20)

##################
# SUBDIRECTORIES #
##################
add_subdirectory(submodules/vk-bootstrap)
add_subdirectory(submodules/VulkanMemoryAllocator)
add_subdirectory(submodules/tinyobjloader)


################
# DEPENDENCIES #
################
find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(EnTT REQUIRED)

add_executable(game-proj vengine/ecs/position.hpp)
target_sources(game-proj PUBLIC
        scenes/test.hpp
        vengine/vengine.hpp
        vengine/event_source.hpp
        vengine/ram_file.hpp
        vengine/io.hpp
        vengine/log.hpp
        vengine/mesh.hpp
        vengine/allocated_buffer.hpp
        vengine/allocated_image.hpp
        vengine/scene.hpp
        vengine/ecs/rotation.hpp
        vengine/ecs/renderable.hpp
        vengine/ecs/velocity.hpp
        vengine/vulkan-utils/pipeline_builder.hpp
        vengine/vulkan-utils/result.hpp
        vengine/vulkan-utils/stringify.hpp
        vengine/vulkan-utils/image_builder.hpp
        vengine/vulkan-utils/image_view_builder.hpp
        vengine/vulkan-utils/render_pass_builder.hpp
        vengine/vulkan-utils/descriptor_set_layout_builder.hpp
        vengine/vulkan-utils/descriptor_pool_builder.hpp
        vengine/vulkan-utils/descriptor_set_updater.hpp)
target_sources(game-proj PRIVATE
        main.cpp
        scenes/test.cpp
        vengine/vengine.cpp
        vengine/ram_file.cpp
        vengine/io.cpp
        vengine/log.cpp
        vengine/mesh.cpp
        vengine/allocated_buffer.cpp
        vengine/allocated_image.cpp
        vengine/scene.cpp)
target_compile_definitions(game-proj PUBLIC GLFW_INCLUDE_VULKAN)
target_link_libraries(game-proj glfw)
target_link_libraries(game-proj glm::glm)
target_link_libraries(game-proj Vulkan::Vulkan)
target_link_libraries(game-proj vk-bootstrap::vk-bootstrap)
target_link_libraries(game-proj VulkanMemoryAllocator)
target_link_libraries(game-proj tinyobjloader)
target_link_libraries(game-proj EnTT::EnTT)

if (MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(game-proj BEFORE PRIVATE /wd4068)
    # else()
    #     # lots of warnings and all warnings as errors
    #     add_compile_options(-Wall -Wextra -pedantic -Werror)
endif ()