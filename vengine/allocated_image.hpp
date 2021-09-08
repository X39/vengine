//
// Created by marco.silipo on 08.09.2021.
//

#ifndef GAME_PROJ_ALLOCATED_IMAGE_HPP
#define GAME_PROJ_ALLOCATED_IMAGE_HPP
#include "vk_mem_alloc.h"

namespace vengine
{
    struct allocated_image
    {
        VkImage image;
        VmaAllocation allocation;
        VmaAllocator allocator;
        allocated_image() : image(nullptr), allocation(nullptr), allocator(nullptr) {}
        explicit allocated_image(VmaAllocator allocator) : image(nullptr), allocation(nullptr), allocator(allocator) {}

        [[nodiscard]] bool uploaded() const { return image || allocator || allocation; }
        void destroy();

        operator VkImage() const { return image; }
        operator VmaAllocator() const { return allocator; }
        operator VmaAllocation() const { return allocation; }
    };
}

#endif //GAME_PROJ_ALLOCATED_IMAGE_HPP
