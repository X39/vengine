//
// Created by marco.silipo on 06.09.2021.
//

#ifndef GAME_PROJ_ALLOCATED_BUFFER_HPP
#define GAME_PROJ_ALLOCATED_BUFFER_HPP
#include "vk_mem_alloc.h"
namespace vengine
{
    struct allocated_buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocator allocator;
        allocated_buffer() : buffer(nullptr), allocation(nullptr), allocator(nullptr) {}
        explicit allocated_buffer(VmaAllocator allocator) : buffer(nullptr), allocation(nullptr), allocator(allocator) {}

        [[nodiscard]] bool uploaded() const { return buffer || allocator || allocation;}
        void destroy();
    };
}

#endif //GAME_PROJ_ALLOCATED_BUFFER_HPP
