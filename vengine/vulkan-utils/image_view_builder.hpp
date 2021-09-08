//
// Created by marco.silipo on 08.09.2021.
//

#ifndef GAME_PROJ_IMAGE_VIEW_BUILDER_HPP
#define GAME_PROJ_IMAGE_VIEW_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "../allocated_image.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class image_view_builder
    {
        std::optional<VkImageUsageFlags> m_image_usage_flags;
        std::optional<VkFormat> m_format;
        std::optional<VkImageAspectFlags> m_image_aspect_flags;
        VkImageViewType m_image_view_type;
        VkImage m_image;
        VkDevice m_device;
        size_t m_mip_level;
        size_t m_array_layers;
        std::optional<VmaMemoryUsage> m_memory_usage;
        VkMemoryPropertyFlags m_memory_property_flags;
    public:
        image_view_builder(VkDevice device, VkImage image)
                : m_device(device),
                m_image(image),
                  m_image_view_type(VK_IMAGE_VIEW_TYPE_2D),
                  m_mip_level(1),
                  m_array_layers(1),
                  m_memory_property_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {

        }

        image_view_builder& set_mip_level(size_t mip_level)
        {
            m_mip_level = mip_level;
            return *this;
        }
        image_view_builder& set_array_layers(size_t array_layers)
        {
            m_array_layers = array_layers;
            return *this;
        }
        image_view_builder& set_image_view_type(VkImageViewType image_view_type)
        {
            m_image_view_type = image_view_type;
            return *this;
        }
        image_view_builder& set_format(VkFormat format)
        {
            m_format = format;
            return *this;
        }
        image_view_builder& set_image_usage(VkImageUsageFlags image_usage_flags)
        {
            m_image_usage_flags = image_usage_flags;
            return *this;
        }
        image_view_builder& set_image_aspect(VkImageAspectFlags image_aspect_flags)
        {
            m_image_aspect_flags = image_aspect_flags;
            return *this;
        }
        image_view_builder& set_memory_usage(VmaMemoryUsage memory_usage)
        {
            m_memory_usage = memory_usage;
            return *this;
        }
        image_view_builder& set_memory_property_flags(VkMemoryPropertyFlags memory_property_flags)
        {
            m_memory_property_flags = memory_property_flags;
            return *this;
        }

        result<VkImageView> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (!m_memory_usage.has_value())
            {
                auto message = "Memory usage never has been set. (set_memory_usage)";
                log::error("vengine::vulkan_utils::image_view_builder::build()", message);
                return message;
            }
            if (!m_image_usage_flags.has_value())
            {
                auto message = "Image usage never has been set. (set_memory_usage)";
                log::error("vengine::vulkan_utils::image_view_builder::set_image_usage()", message);
                return message;
            }
            if (!m_image_aspect_flags.has_value())
            {
                auto message = "Image aspect never has been set. (set_image_aspect)";
                log::error("vengine::vulkan_utils::image_view_builder::set_image_usage()", message);
                return message;
            }
            if (!m_format.has_value())
            {
                auto message = "Format never has been set. (set_format)";
                log::error("vengine::vulkan_utils::image_view_builder::build()", message);
                return message;
            }
            if (m_mip_level > UINT32_MAX)
            {
                auto message = "Mip level exceeds the value that vulkan can handle.";
                log::error("vengine::vulkan_utils::image_view_builder::build()", message);
                return message;
            }
            if (m_array_layers > UINT32_MAX)
            {
                auto message = "Array layers exceed the value that vulkan can handle.";
                log::error("vengine::vulkan_utils::image_view_builder::build()", message);
                return message;
            }

            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.pNext = nullptr;

            image_view_create_info.viewType = m_image_view_type;
            image_view_create_info.image = m_image;
            image_view_create_info.format = m_format.value();
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = (uint32_t)m_mip_level;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = (uint32_t)m_array_layers;
            image_view_create_info.subresourceRange.aspectMask = m_image_aspect_flags.value();



            VkImageView result = {};
            auto create_image_view_result = vkCreateImageView(
                    m_device,
                    &image_view_create_info,
                    nullptr,
                    &result);
            if (create_image_view_result == VK_SUCCESS)
            {
                return { result };
            }
            else
            {
                auto message = std::string("Failed to build image view (").append(stringify::data(create_image_view_result)).append(").");
                log::error("vengine::vulkan_utils::image_view_builder::build()", message);
                return { create_image_view_result, message };
            }
        }
    };
}

#endif //GAME_PROJ_IMAGE_VIEW_BUILDER_HPP
