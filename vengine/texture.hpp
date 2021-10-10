//
// Created by marco.silipo on 10.10.2021.
//

#ifndef GAME_PROJ_TEXTURE_HPP
#define GAME_PROJ_TEXTURE_HPP

#include "vulkan-utils/result.hpp"
#include "allocated_image.hpp"
#include "ram_file.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <cstdint>
#include "vk_mem_alloc.h"


namespace vengine
{
    class vengine;
    struct texture
    {
        std::vector<uint8_t> rgba_data;
        size_t width;
        size_t height;

        allocated_image image_buffer;

        texture() = default;
        [[nodiscard]] static std::optional<texture> from_ram_file(const ram_file& file);

        [[nodiscard]] vulkan_utils::result<void> upload_to_cpu_writable_gpu_memory(VmaAllocator allocator);
        [[nodiscard]] vulkan_utils::result<void> upload_to_gpu_memory(::vengine::vengine& engine, VmaAllocator allocator);
        [[nodiscard]] bool uploaded() const { return image_buffer.uploaded(); }
        void destroy();
        [[nodiscard]] size_t size() const { return rgba_data.size(); }
        [[nodiscard]] VkExtent3D extent3d() const {
            VkExtent3D data;
            data.width = static_cast<uint32_t>(width);
            data.height = static_cast<uint32_t>(height);
            data.depth = 1;
            return data;
        }
        [[nodiscard]] VkExtent2D extent2d() const {
            VkExtent2D data;
            data.width = static_cast<uint32_t>(width);
            data.height = static_cast<uint32_t>(height);
            return data;
        }
    };
}


#endif //GAME_PROJ_TEXTURE_HPP
