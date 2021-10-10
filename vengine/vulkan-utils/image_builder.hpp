//
// Created by marco.silipo on 08.09.2021.
//

#ifndef GAME_PROJ_IMAGE_BUILDER_HPP
#define GAME_PROJ_IMAGE_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "../allocated_image.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class image_builder
    {
        VmaAllocator m_allocator;
        std::optional<VkImageUsageFlags> m_image_usage_flags;
        std::optional<VkFormat> m_format;
        VkImageType m_image_type;
        VkExtent3D m_extent;
        size_t m_mip_level;
        size_t m_array_layers;
        std::optional<VmaMemoryUsage> m_memory_usage;
        VkMemoryPropertyFlags m_memory_property_flags;
    public:
        image_builder(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t depth)
                : m_allocator(allocator),
                  m_extent({ }),
                  m_image_type(VK_IMAGE_TYPE_2D),
                  m_mip_level(1),
                  m_array_layers(1),
                  m_memory_property_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            m_extent.width = width;
            m_extent.height = height;
            m_extent.depth = depth;
        }
        image_builder(VmaAllocator allocator, VkExtent3D extent)
                : m_allocator(allocator),
                  m_extent(extent),
                  m_image_type(VK_IMAGE_TYPE_2D),
                  m_mip_level(1),
                  m_array_layers(1),
                  m_memory_property_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {

        }

        image_builder& set_mip_level(size_t mip_level)
        {
            m_mip_level = mip_level;
            return *this;
        }
        image_builder& set_array_layers(size_t array_layers)
        {
            m_array_layers = array_layers;
            return *this;
        }
        image_builder& set_image_type(VkImageType image_type)
        {
            m_image_type = image_type;
            return *this;
        }
        image_builder& set_format(VkFormat format)
        {
            m_format = format;
            return *this;
        }
        image_builder& set_image_usage(VkImageUsageFlags image_usage_flags)
        {
            m_image_usage_flags = image_usage_flags;
            return *this;
        }
        image_builder& set_memory_usage(VmaMemoryUsage memory_usage)
        {
            m_memory_usage = memory_usage;
            return *this;
        }
        image_builder& set_memory_property_flags(VkMemoryPropertyFlags memory_property_flags)
        {
            m_memory_property_flags = memory_property_flags;
            return *this;
        }

        result<allocated_image> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (!m_memory_usage.has_value())
            {
                auto message = "Memory usage never has been set. (set_memory_usage)";
                log::error("vengine::vulkan_utils::buffer_builder::build()", message);
                return message;
            }
            if (!m_image_usage_flags.has_value())
            {
                auto message = "Image usage never has been set. (set_memory_usage)";
                log::error("vengine::vulkan_utils::image_builder::set_image_usage()", message);
                return message;
            }
            if (!m_format.has_value())
            {
                auto message = "Format never has been set. (set_format)";
                log::error("vengine::vulkan_utils::image_builder::build()", message);
                return message;
            }
            if (m_mip_level > UINT32_MAX)
            {
                auto message = "Mip level exceeds the value that vulkan can handle.";
                log::error("vengine::vulkan_utils::image_builder::build()", message);
                return message;
            }
            if (m_array_layers > UINT32_MAX)
            {
                auto message = "Array layers exceed the value that vulkan can handle.";
                log::error("vengine::vulkan_utils::image_builder::build()", message);
                return message;
            }


            VkImageCreateInfo image_create_info = { };
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = nullptr;
            image_create_info.imageType = m_image_type;
            image_create_info.format = m_format.value();
            image_create_info.extent = m_extent;
            image_create_info.mipLevels = (uint32_t)m_mip_level;
            image_create_info.arrayLayers = (uint32_t)m_array_layers;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = m_image_usage_flags.value();


            VmaAllocationCreateInfo allocation_create_info = {};
            allocation_create_info.usage = m_memory_usage.value();
            allocation_create_info.requiredFlags = m_memory_property_flags;

            allocated_image result(m_allocator);
            auto create_image_result = vmaCreateImage(
                    m_allocator,
                    &image_create_info,
                    &allocation_create_info,
                    &result.image,
                    &result.allocation,
                    nullptr);
            if (create_image_result == VK_SUCCESS)
            {
                return { result };
            }
            else
            {
                auto message = std::string("Failed to build allocated image (").append(stringify::data(create_image_result)).append(").");
                log::error("vengine::vulkan_utils::image_builder::build()", message);
                return { create_image_result, message };
            }
        }
    };
}

#endif //GAME_PROJ_IMAGE_BUILDER_HPP
