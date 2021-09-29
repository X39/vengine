//
// Created by marco.silipo on 06.09.2021.
//

#include "mesh.hpp"
#include "log.hpp"
#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/buffer_builder.hpp"
#include "vengine.hpp"


#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

vengine::vulkan_utils::result<void> vengine::mesh::upload_to_cpu_writable_gpu_memory(VmaAllocator allocator)
{
    if (vertex_buffer.uploaded())
    {
        log::warning("vengine::mesh::upload_to_cpu_writable_gpu_memory(VmaAllocator)", "Attempt was made to upload a mesh twice to the GPU.");
        return { VK_SUCCESS, "Attempt was made to upload_to_cpu_writable_gpu_memory a mesh twice to the GPU." };
    }

    auto buffer_builder_result = vulkan_utils::buffer_builder(allocator, size())
            .set_buffer_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .build();
    if (!buffer_builder_result.good())
    {
        return buffer_builder_result;
    }
    vertex_buffer = buffer_builder_result.value();

    return vertex_buffer.with_mapped([&](auto& span) {
        memcpy(span.data(), vertices.data(), vertices.size() * sizeof(vertex));
    });
}

vengine::vulkan_utils::result<void> vengine::mesh::upload_to_gpu_memory(::vengine::vengine& engine, VmaAllocator allocator)
{
    if (vertex_buffer.uploaded())
    {
        log::warning("vengine::mesh::upload_to_gpu_memory(vengine&, VmaAllocator)", "Attempt was made to upload a mesh twice to the GPU.");
        return { VK_SUCCESS, "Attempt was made to upload a mesh twice to the GPU." };
    }

    auto cpu_writeable_buffer_builder_result = vulkan_utils::buffer_builder(allocator, size())
            .set_buffer_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .build();
    if (!cpu_writeable_buffer_builder_result.good())
    {
        return cpu_writeable_buffer_builder_result;
    }
    auto tmp = cpu_writeable_buffer_builder_result.value();

    tmp.with_mapped([&](auto& span) {
        memcpy(span.data(), vertices.data(), vertices.size() * sizeof(vertex));
    });


    auto gpu_buffer_builder_result = vulkan_utils::buffer_builder(allocator, size())
            .set_buffer_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
            .build();
    if (!gpu_buffer_builder_result.good())
    {
        tmp.destroy();
        return gpu_buffer_builder_result;
    }
    vertex_buffer = gpu_buffer_builder_result.value();

    auto execute_result = engine.execute([&](auto& command_buffer) {
        VkBufferCopy buffer_copy{};
        buffer_copy.size = size();
        vkCmdCopyBuffer(command_buffer, tmp.buffer, vertex_buffer.buffer, 1, &buffer_copy);
    });
    if (!execute_result.good())
    {
        tmp.destroy();
        vertex_buffer.destroy();
        return execute_result;
    }
    tmp.destroy();
    return {};
}

std::optional<vengine::mesh> vengine::mesh::from_obj(const ram_file& obj_file, const ram_file& mtl_file)
{
    // Move ram_files into string
    std::string data(obj_file.begin(), obj_file.end());
    std::string mtl_text(mtl_file.begin(), mtl_file.end());

    //load the OBJ file
    tinyobj::ObjReader obj_reader;
    tinyobj::ObjReaderConfig obj_reader_config;

    if (!obj_reader.ParseFromString(data, mtl_text, obj_reader_config))
    {
        std::string message("Failed to read in object: ");
        message.append(obj_reader.Error());
        log::error("vengine::mesh::from_obj(const ram_file&, const ram_file&)", message);
        return {};
    }
    if (!obj_reader.Warning().empty())
    {
        std::string message("Warning was reported when reading in obj file: ");
        message.append(obj_reader.Warning());
        log::warning("vengine::mesh::from_obj(const ram_file&, const ram_file&)", message);
    }

    auto shapes = obj_reader.GetShapes();
    auto attrib = obj_reader.GetAttrib();

    mesh out_mesh;
    for (auto & shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            auto num_face_vertices = shape.mesh.num_face_vertices[f];

            for (size_t face_vertex_index = 0; face_vertex_index < num_face_vertices; face_vertex_index++) {
                tinyobj::index_t indices = shape.mesh.indices[index_offset + face_vertex_index];

                // Positions
                tinyobj::real_t vx = attrib.vertices[3 * indices.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * indices.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * indices.vertex_index + 2];

                // Normals
                tinyobj::real_t nx = attrib.normals[3 * indices.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * indices.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * indices.normal_index + 2];

                vertex new_vert{ { vx, vy, vz },  {nx, ny, nz}, {}};

                // For debugging - Hardcode color to normal map
                new_vert.color = new_vert.normal;

                out_mesh.vertices.push_back(new_vert);
            }
            index_offset += num_face_vertices;
        }
    }

    return out_mesh;
}

void vengine::mesh::destroy()
{
    vertex_buffer.destroy();
}
