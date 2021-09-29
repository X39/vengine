//
// Created by marco.silipo on 06.09.2021.
//

#ifndef GAME_PROJ_ALLOCATED_BUFFER_HPP
#define GAME_PROJ_ALLOCATED_BUFFER_HPP
#include "vk_mem_alloc.h"
#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/result.hpp"
#include "log.hpp"

#include <span>
#include <functional>
#include <string>
namespace vengine
{
    struct allocated_buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocator allocator;
        size_t size;
        allocated_buffer() : buffer(nullptr), allocation(nullptr), allocator(nullptr), size(0) {}
        explicit allocated_buffer(VmaAllocator allocator) : buffer(nullptr), allocation(nullptr), allocator(allocator), size(0) {}

        [[nodiscard]] bool uploaded() const { return buffer || allocator || allocation; }
        void destroy();

        vulkan_utils::result<void> with_mapped(const std::function<void(std::span<uint8_t>&)>& func) const
        {
            void* data{};
            auto map_memory_result = vmaMapMemory(allocator, allocation, &data);
            if (map_memory_result != VK_SUCCESS)
            {
                auto message = std::string("Failed to map memory (").append(vulkan_utils::stringify::data(map_memory_result)).append(")");
                log::error("vengine::allocated_buffer::with_mapped(const std::function<void(std::span<uint8_t>&)>&)", message);
                return { map_memory_result, message };
            }

            std::span span{ reinterpret_cast<uint8_t*>(data), size };
            func(span);

            vmaUnmapMemory(allocator, allocation);
            return {};
        }
    };
}

#endif //GAME_PROJ_ALLOCATED_BUFFER_HPP
