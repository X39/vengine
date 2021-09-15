//
// Created by marco.silipo on 06.09.2021.
//

#include "allocated_buffer.hpp"

void vengine::allocated_buffer::destroy()
{
    if (!uploaded())
    {
        return;
    }
    vmaDestroyBuffer(allocator, buffer, allocation);
    allocator = nullptr;
    buffer = nullptr;
    allocation = nullptr;
    size = 0;
}
