cmake_minimum_required(VERSION 3.9)
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

add_executable(game-proj
        vengine/vengine.hpp
        vengine/event_source.hpp
        vengine/vengine.cpp
        vengine/ram_file.cpp
        vengine/ram_file.hpp
        vengine/io.hpp
        vengine/io.cpp
        vengine/vulkan-utils/pipeline_builder.hpp
        vengine/vulkan-utils/result.hpp
        vengine/log.hpp
        vengine/log.cpp
        vengine/vulkan-utils/stringify.hpp
        vengine/mesh.cpp
        vengine/mesh.hpp
        vengine/allocated_buffer.hpp
        vengine/allocated_buffer.cpp vengine/allocated_image.cpp vengine/allocated_image.hpp vengine/vulkan-utils/image_builder.hpp vengine/vulkan-utils/image_view_builder.hpp vengine/vulkan-utils/render_pass_builder.hpp vengine/scene.cpp vengine/scene.hpp vengine/vulkan-utils/descriptor_set_layout_builder.hpp vengine/vulkan-utils/descriptor_pool_builder.hpp vengine/ecs/position.hpp vengine/ecs/velocity.hpp scenes/test.cpp scenes/test.hpp)
target_sources(game-proj PRIVATE main.cpp)
target_compile_definitions(game-proj PUBLIC GLFW_INCLUDE_VULKAN)
target_link_libraries(game-proj glfw)
target_link_libraries(game-proj glm::glm)
target_link_libraries(game-proj Vulkan::Vulkan)
target_link_libraries(game-proj vk-bootstrap::vk-bootstrap)
target_link_libraries(game-proj VulkanMemoryAllocator)
target_link_libraries(game-proj tinyobjloader)
target_link_libraries(game-proj EnTT::EnTT)