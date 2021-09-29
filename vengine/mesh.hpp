//
// Created by marco.silipo on 06.09.2021.
//

#ifndef GAME_PROJ_MESH_HPP
#define GAME_PROJ_MESH_HPP

#include "vulkan-utils/result.hpp"
#include "allocated_buffer.hpp"
#include "ram_file.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "vk_mem_alloc.h"

namespace vengine
{
    class vengine;
    struct vertex_input_description
    {
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        VkPipelineVertexInputStateCreateFlags vertex_input_state_create_flags = 0;
    };

#pragma pack(push, 1)
    struct vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;

        vertex() = default;
        explicit vertex(glm::vec3 position) : position(position), normal({  }), color({ }) {}
        vertex(glm::vec3 position, glm::vec3 normal) : position(position), normal(normal), color({ }) {}
        vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 color) : position(position), normal(normal), color(color) {}

        static vertex_input_description get_vertex_input_description()
        {
            vertex_input_description description;

            VkVertexInputBindingDescription mainBinding = {};
            mainBinding.binding = 0;
            mainBinding.stride = sizeof(vertex);
            mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            description.binding_descriptions.push_back(mainBinding);

            VkVertexInputAttributeDescription position_input_attribute_description = {};
            position_input_attribute_description.binding = 0;
            position_input_attribute_description.location = 0;
            position_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
            position_input_attribute_description.offset = offsetof(vertex, position);
            description.attribute_descriptions.push_back(position_input_attribute_description);

            VkVertexInputAttributeDescription normal_input_attribute_description = {};
            normal_input_attribute_description.binding = 0;
            normal_input_attribute_description.location = 1;
            normal_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
            normal_input_attribute_description.offset = offsetof(vertex, normal);
            description.attribute_descriptions.push_back(normal_input_attribute_description);

            VkVertexInputAttributeDescription color_input_attribute_description = {};
            color_input_attribute_description.binding = 0;
            color_input_attribute_description.location = 2;
            color_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
            color_input_attribute_description.offset = offsetof(vertex, color);

            description.attribute_descriptions.push_back(color_input_attribute_description);
            return description;
        }
    };
#pragma pack(pop)

    struct mesh {
#pragma pack(push, 1)
        struct push_constant
        {
            glm::vec4 data;
            glm::mat4 matrix;
        };
#pragma pack(pop)
        std::vector<vertex> vertices;

        allocated_buffer vertex_buffer;

        mesh() = default;
        mesh(std::initializer_list<vertex> vertexes) : vertices(vertexes.begin(), vertexes.end()) {}


        [[nodiscard]] vulkan_utils::result<void> upload_to_cpu_writable_gpu_memory(VmaAllocator allocator);
        [[nodiscard]] vulkan_utils::result<void> upload_to_gpu_memory(::vengine::vengine& engine, VmaAllocator allocator);
        [[nodiscard]] bool uploaded() const { return vertex_buffer.uploaded(); }
        void destroy();
        [[nodiscard]] static std::optional<mesh> from_obj(const ram_file& obj_file, const ram_file& mtl_file);
        [[nodiscard]] size_t size() const { return vertices.size() * sizeof(vertex); }
    };
}


#endif //GAME_PROJ_MESH_HPP
