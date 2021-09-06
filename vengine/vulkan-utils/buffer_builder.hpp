//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_BUFFER_BUILDER_HPP
#define GAME_PROJ_BUFFER_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "../allocated_buffer.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class buffer_builder
    {
        VmaAllocator m_allocator;
        std::optional<VmaMemoryUsage> m_memory_usage;
        std::optional<VkBufferUsageFlags> m_buffer_usage;
        size_t m_size;
    public:
        buffer_builder(VmaAllocator allocator, size_t size)
                : m_allocator(allocator),
                m_size(size)
        {

        }

        buffer_builder& set_memory_usage(VmaMemoryUsage memory_usage)
        {
            m_memory_usage = memory_usage;
            return *this;
        }
        buffer_builder& set_buffer_usage(VkBufferUsageFlags buffer_usage_flags)
        {
            m_buffer_usage = buffer_usage_flags;
            return *this;
        }

        result<allocated_buffer> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (!m_memory_usage.has_value())
            {
                auto message = "Memory usage never has been set. (set_memory_usage)";
                log::error("vengine::vulkan_utils::buffer_builder::build()", message);
                return message;
            }
            if (!m_buffer_usage.has_value())
            {
                auto message = "Buffer usage never has been set. (set_buffer_usage)";
                log::error("vengine::vulkan_utils::buffer_builder::build()", message);
                return message;
            }
            
            
            VkBufferCreateInfo buffer_create_info = {};
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.size = m_size;
            buffer_create_info.usage = m_buffer_usage.value();


            VmaAllocationCreateInfo allocation_create_info = {};
            allocation_create_info.usage = m_memory_usage.value();

            allocated_buffer result(m_allocator);
            auto create_buffer_result = vmaCreateBuffer(m_allocator, &buffer_create_info, &allocation_create_info,
                    &result.buffer,
                    &result.allocation,
                    nullptr);
            if (create_buffer_result == VK_SUCCESS)
            {
                return { result };
            }
            else
            {
                auto message = std::string("Failed to build allocated buffer (").append(stringify::data(create_buffer_result)).append(").");
                log::error("vengine::vulkan_utils::buffer_builder::build()", message);
                return { create_buffer_result, message };
            }
        }
    };
}

#endif //GAME_PROJ_BUFFER_BUILDER_HPP
