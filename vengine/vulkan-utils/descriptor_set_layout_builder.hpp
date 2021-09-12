//
// Created by marco.silipo on 10.09.2021.
//

#ifndef GAME_PROJ_DESCRIPTOR_SET_LAYOUT_BUILDER_HPP
#define GAME_PROJ_DESCRIPTOR_SET_LAYOUT_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "stringify.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class descriptor_set_layout_builder
    {
        VkDevice m_device;
        std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
        VkDescriptorSetLayoutCreateFlags m_descriptor_set_layout_create_flags;
    public:
        explicit descriptor_set_layout_builder(VkDevice device)
                : m_device(device), m_descriptor_set_layout_create_flags(0)
        {

        }

        descriptor_set_layout_builder& add_layout_binding(size_t binding, VkDescriptorType descriptor_type, size_t descriptor_count, VkShaderStageFlags stage_flags)
        {
            if (binding > UINT32_MAX)
            {
                auto message = "Binding is outside of supported range for vulkan.";
                log::warning("vengine::vulkan_utils::descriptor_pool_builder::add_layout_binding(size_t, VkDescriptorType, size_t, VkShaderStageFlags)", message);
                binding = UINT32_MAX;
            }
            if (descriptor_count > UINT32_MAX)
            {
                auto message = "Descriptor count is outside of supported range for vulkan.";
                log::warning("vengine::vulkan_utils::descriptor_pool_builder::add_layout_binding(size_t, VkDescriptorType, size_t, VkShaderStageFlags)", message);
                descriptor_count = UINT32_MAX;
            }
            VkDescriptorSetLayoutBinding layout_binding;
            layout_binding.binding = (uint32_t)binding;
            layout_binding.descriptorCount = (uint32_t)descriptor_count;
            layout_binding.descriptorType = descriptor_type;
            layout_binding.stageFlags = stage_flags;

            m_descriptor_set_layout_bindings.push_back(layout_binding);
            return *this;
        }

        descriptor_set_layout_builder& set_descriptor_set_layout_create_flags(VkDescriptorSetLayoutCreateFlags flags)
        {
            m_descriptor_set_layout_create_flags = flags;
            return *this;
        }

        result<VkDescriptorSetLayout> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_descriptor_set_layout_bindings.size() > UINT32_MAX)
            {
                auto message = "More descriptor set layout bindings have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::descriptor_set_layout_builder::build()", message);
                return message;
            }

            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.pNext = nullptr;
            descriptor_set_layout_create_info.flags = m_descriptor_set_layout_create_flags;
            descriptor_set_layout_create_info.bindingCount = (uint32_t)m_descriptor_set_layout_bindings.size();
            descriptor_set_layout_create_info.pBindings = m_descriptor_set_layout_bindings.data();

            VkDescriptorSetLayout descriptor_set_layout;
            auto descriptor_set_layout_creation_result = vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);
            if (descriptor_set_layout_creation_result == VK_SUCCESS)
            {
                return { descriptor_set_layout };
            }
            else
            {
                auto message = std::string("Failed to build descriptor set layout (").append(stringify::data(descriptor_set_layout_creation_result)).append(")");
                log::error("vengine::vulkan_utils::descriptor_set_layout_builder::build()", message);
                return { descriptor_set_layout_creation_result };
            }
        }
    };
}

#endif //GAME_PROJ_DESCRIPTOR_SET_LAYOUT_BUILDER_HPP
