//
// Created by marco.silipo on 10.09.2021.
//

#ifndef GAME_PROJ_DESCRIPTOR_POOL_BUILDER_HPP
#define GAME_PROJ_DESCRIPTOR_POOL_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "stringify.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class descriptor_pool_builder
    {
        VkDevice m_device;
        std::vector<VkDescriptorPoolSize> m_descriptor_pool_sizes;
        size_t m_reserved_sets;
    public:
        explicit descriptor_pool_builder(VkDevice device, size_t reserved_sets)
                : m_device(device), m_reserved_sets(reserved_sets)
        {

        }

        descriptor_pool_builder& add_layout_binding(VkDescriptorType descriptor_type, size_t descriptor_count)
        {
            if (descriptor_count > UINT32_MAX)
            {
                auto message = "Descriptor count is outside of supported range for vulkan.";
                log::warning("vengine::vulkan_utils::descriptor_pool_builder::add_layout_binding(VkDescriptorType, size_t)", message);
                descriptor_count = UINT32_MAX;
            }
            VkDescriptorPoolSize descriptor_pool_size = {};
            descriptor_pool_size.type = descriptor_type;
            descriptor_pool_size.descriptorCount = (uint32_t)descriptor_count;

            m_descriptor_pool_sizes.push_back(descriptor_pool_size);
            return *this;
        }
        result<VkDescriptorPool> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_reserved_sets > UINT32_MAX)
            {
                auto message = "More sets have been requested to be reserved then vulkan can handle.";
                log::error("vengine::vulkan_utils::descriptor_pool_builder::build()", message);
                return message;
            }
            if (m_descriptor_pool_sizes.size() > UINT32_MAX)
            {
                auto message = "More descriptor pool sizes have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::descriptor_pool_builder::build()", message);
                return message;
            }
            VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
            descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptor_pool_create_info.flags = 0;
            descriptor_pool_create_info.maxSets = (uint32_t)m_reserved_sets;
            descriptor_pool_create_info.poolSizeCount = (uint32_t)m_descriptor_pool_sizes.size();
            descriptor_pool_create_info.pPoolSizes = m_descriptor_pool_sizes.data();

            VkDescriptorPool descriptor_pool;
            auto descriptor_pool_creation_result = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
            if (descriptor_pool_creation_result == VK_SUCCESS)
            {
                return { descriptor_pool };
            }
            else
            {
                auto message = std::string("Failed to build descriptor set layout (").append(stringify::data(descriptor_pool_creation_result)).append(")");
                log::error("vengine::vulkan_utils::descriptor_pool_builder::build()", message);
                return { descriptor_pool_creation_result };
            }
        }
    };
}


#endif //GAME_PROJ_DESCRIPTOR_POOL_BUILDER_HPP
