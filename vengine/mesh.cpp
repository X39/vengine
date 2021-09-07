//
// Created by marco.silipo on 06.09.2021.
//

#include "mesh.hpp"
#include "log.hpp"
#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/buffer_builder.hpp"


#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

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
