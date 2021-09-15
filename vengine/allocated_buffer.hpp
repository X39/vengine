//
// Created by marco.silipo on 06.09.2021.
//

#ifndef GAME_PROJ_ALLOCATED_BUFFER_HPP
#define GAME_PROJ_ALLOCATED_BUFFER_HPP
#include "vk_mem_alloc.h"

#include <span>
#include <functional>
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

        void with_mapped(const std::function<void(std::span<uint8_t>&)>& func) const
        {
            void* data{};
            vmaMapMemory(allocator, allocation, &data);

            std::span span{ reinterpret_cast<uint8_t*>(data), size };
            func(span);

            vmaUnmapMemory(allocator, allocation);
        }
    };
}

#endif //GAME_PROJ_ALLOCATED_BUFFER_HPP
