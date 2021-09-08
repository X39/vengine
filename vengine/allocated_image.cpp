//
// Created by marco.silipo on 08.09.2021.
//

#include "allocated_image.hpp"

void vengine::allocated_image::destroy()
{
    if (!uploaded())
    {
        return;
    }
    vmaDestroyImage(allocator, image, allocation);
    allocator = nullptr;
    image = nullptr;
    allocation = nullptr;
}
