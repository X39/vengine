//
// Created by marco.silipo on 06.09.2021.
//

#include "mesh.hpp"
#include <vulkan/vulkan.h>
#include "log.hpp"
#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/buffer_builder.hpp"

void vengine::mesh::upload(VmaAllocator allocator)
{
    if (vertex_buffer.uploaded())
    {
        log::warning("vengine::mesh::upload(VmaAllocator)", "Attempt was made to upload a mesh twice to the GPU.");
        return;
    }

    auto buffer_builder_result = vulkan_utils::buffer_builder(allocator, size())
            .set_buffer_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .build();
    if (!buffer_builder_result.has_value())
    {
        log::error("vengine::mesh::upload(VmaAllocator)", "Failed to create buffer.");
        return;
    }
    vertex_buffer = buffer_builder_result.value();

    void* data;
    auto map_memory_result = vmaMapMemory(allocator, vertex_buffer.allocation, &data);
    if (map_memory_result != VK_SUCCESS)
    {
        auto message = std::string("Failed to map memory for uploading mesh (").append(vulkan_utils::stringify::data(map_memory_result)).append(").");
        log::error("vengine::mesh::upload(VmaAllocator)", message);
        return;
    }

    memcpy(data, vertices.data(), vertices.size() * sizeof(vertex));

    vmaUnmapMemory(allocator, vertex_buffer.allocation);
}
