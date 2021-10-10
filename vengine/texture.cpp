//
// Created by marco.silipo on 10.10.2021.
//

#include "log.hpp"
#include "texture.hpp"
#include <stb_image.h>


#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/buffer_builder.hpp"
#include "vulkan-utils/image_builder.hpp"
#include "vengine.hpp"


#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

std::optional<vengine::texture> vengine::texture::from_ram_file(const ram_file &file)
{
    const char *source = "vengine::texture::from_ram_file(const ram_file&)";
    const size_t rgba_size = 4;

    if (file.size() > INT32_MAX)
    {
        const char *message = "Cannot load ram_file as size exceeded INT32_MAX.";
        log::warning(source, message);
        return { };
    }

    int width, height, texture_channels;
    stbi_uc *pixels = stbi_load_from_memory(
            file.data(), (int32_t) file.size(), &width, &height, &texture_channels, STBI_rgb_alpha);
    if (!pixels)
    {
        const char
                *message = "Attempt was made to load a texture from a ram_file but loading the image from it failed.";
        log::warning(source, message);
        return { };
    }

    size_t image_size = (size_t) width * (size_t) height * rgba_size;
    texture result;
    result.width = width;
    result.height = height;
    result.rgba_data.resize(image_size);
    result.rgba_data.insert(result.rgba_data.begin(), pixels, pixels + image_size);


    return std::optional<texture>();
}


vengine::vulkan_utils::result<void>
vengine::texture::upload_to_gpu_memory(::vengine::vengine &engine, VmaAllocator allocator)
{
    const char *source = "vengine::texture::upload_to_gpu_memory(::vengine::vengine, VmaAllocator)";
    if (image_buffer.uploaded())
    {
        const char *message = "Attempt was made to upload an image twice to the GPU.";
        log::warning(source, message);
        return { VK_SUCCESS, message };
    }

    auto cpu_writeable_buffer_builder_result = vulkan_utils::buffer_builder(allocator, size()).set_buffer_usage(
                                                                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                                                                                              .set_memory_usage(
                                                                                                      VMA_MEMORY_USAGE_CPU_TO_GPU)
                                                                                              .build();
    if (!cpu_writeable_buffer_builder_result.good())
    {
        return cpu_writeable_buffer_builder_result;
    }
    auto tmp = cpu_writeable_buffer_builder_result.value();

    tmp.with_mapped(
            [&](auto &span)
            {
                memcpy(span.data(), rgba_data.data(), size());
            });


    // Create Image
    {
        auto gpu_image_builder_result = vulkan_utils::image_builder(
                allocator, extent3d()).set_image_usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                      .set_format(VK_FORMAT_R8G8B8A8_SRGB)
                                      .set_memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                                      .build();
        if (!gpu_image_builder_result.good())
        {
            tmp.destroy();
            return gpu_image_builder_result;
        }
        image_buffer = gpu_image_builder_result.value();
    }
    auto execute_result = engine.execute(
            [&](auto &command_buffer)
            {
                {
                    VkImageSubresourceRange image_subresource_range = { };
                    image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    image_subresource_range.baseMipLevel = 0;
                    image_subresource_range.levelCount = 1;
                    image_subresource_range.baseArrayLayer = 0;
                    image_subresource_range.layerCount = 1;

                    VkImageMemoryBarrier transfer_image_memory_barrier = { };
                    transfer_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

                    transfer_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    transfer_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    transfer_image_memory_barrier.image = image_buffer.image;
                    transfer_image_memory_barrier.subresourceRange = image_subresource_range;

                    transfer_image_memory_barrier.srcAccessMask = 0;
                    transfer_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    // Barrier the image into the transfer-receive layout
                    vkCmdPipelineBarrier(
                            command_buffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            0,
                            0,
                            nullptr,
                            0,
                            nullptr,
                            1,
                            &transfer_image_memory_barrier);
                }
                {
                    VkBufferImageCopy buffer_image_copy = { };
                    buffer_image_copy.bufferOffset = 0;
                    buffer_image_copy.bufferRowLength = 0;
                    buffer_image_copy.bufferImageHeight = 0;

                    buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    buffer_image_copy.imageSubresource.mipLevel = 0;
                    buffer_image_copy.imageSubresource.baseArrayLayer = 0;
                    buffer_image_copy.imageSubresource.layerCount = 1;
                    buffer_image_copy.imageExtent = extent3d();

                    // Copy the data inside the buffer into the image
                    vkCmdCopyBufferToImage(
                            command_buffer,
                            tmp.buffer,
                            image_buffer.image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &buffer_image_copy);
                }

                {
                    VkImageSubresourceRange image_subresource_range = { };
                    image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    image_subresource_range.baseMipLevel = 0;
                    image_subresource_range.levelCount = 1;
                    image_subresource_range.baseArrayLayer = 0;
                    image_subresource_range.layerCount = 1;

                    VkImageMemoryBarrier readable_image_memory_barrier = { };
                    readable_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

                    readable_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    readable_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    readable_image_memory_barrier.image = image_buffer.image;
                    readable_image_memory_barrier.subresourceRange = image_subresource_range;

                    readable_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    readable_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    //barrier the image into the shader readable layout
                    vkCmdPipelineBarrier(
                            command_buffer,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0,
                            0,
                            nullptr,
                            0,
                            nullptr,
                            1,
                            &readable_image_memory_barrier);
                }

            });
    if (!execute_result.good())
    {
        tmp.destroy();
        image_buffer.destroy();
        return execute_result;
    }
    tmp.destroy();
    return { };
}

void vengine::texture::destroy()
{
    image_buffer.destroy();
}
