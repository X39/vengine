//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_PIPELINE_LAYOUT_BUILDER_HPP
#define GAME_PROJ_PIPELINE_LAYOUT_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "stringify.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class pipeline_layout_builder
    {
        VkDevice m_device;
        std::vector<VkPushConstantRange> m_push_constant_ranges;
        std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    public:
        explicit pipeline_layout_builder(VkDevice device)
                : m_device(device)
        {

        }

        pipeline_layout_builder& add_push_constant_range(VkPushConstantRange push_constant_range)
        {
            m_push_constant_ranges.push_back(push_constant_range);
            return *this;
        }
        pipeline_layout_builder& add_push_constant_range(size_t size, size_t offset, VkShaderStageFlagBits stage_flags)
        {
            if (size > UINT32_MAX)
            {
                auto message = "Size is outside of supported range for vulkan.";
                log::warning("vengine::vulkan_utils::pipeline_layout_builder::add_push_constant_range(size_t, size_t, VkShaderStageFlagBits)", message);
                size = UINT32_MAX;
            }
            if (offset > UINT32_MAX)
            {
                auto message = "Offset is outside of supported range for vulkan.";
                log::warning("vengine::vulkan_utils::pipeline_layout_builder::add_push_constant_range(size_t, size_t, VkShaderStageFlagBits)", message);
                offset = UINT32_MAX;
            }
            VkPushConstantRange push_constant_range = {};
            push_constant_range.size = (uint32_t)size;
            push_constant_range.offset = (uint32_t)offset;
            push_constant_range.stageFlags = stage_flags;
            m_push_constant_ranges.push_back(push_constant_range);
            return *this;
        }
        pipeline_layout_builder& add_descriptor_set_layout(VkDescriptorSetLayout descriptor_set_layout)
        {
            m_descriptor_set_layouts.push_back(descriptor_set_layout);
            return *this;
        }

        result<VkPipelineLayout> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_push_constant_ranges.size() > UINT32_MAX)
            {
                auto message = "More push constant ranges have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_layout_builder::build()", message);
                return message;
            }
            if (m_descriptor_set_layouts.size() > UINT32_MAX)
            {
                auto message = "More descriptor set layouts have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_layout_builder::build()", message);
                return message;
            }
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;

            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = (uint32_t)m_descriptor_set_layouts.size();
            pipeline_layout_create_info.pSetLayouts = m_descriptor_set_layouts.data();
            pipeline_layout_create_info.pushConstantRangeCount = (uint32_t)m_push_constant_ranges.size();
            pipeline_layout_create_info.pPushConstantRanges = m_push_constant_ranges.data();

            VkPipelineLayout pipeline_layout;
            auto pipeline_layout_creation_result = vkCreatePipelineLayout(m_device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
            if (pipeline_layout_creation_result == VK_SUCCESS)
            {
                return { pipeline_layout };
            }
            else
            {
                auto message = std::string("Failed to build pipeline layout (").append(stringify::data(pipeline_layout_creation_result)).append(")");
                log::error("vengine::vulkan_utils::pipeline_layout_builder::build()", message);
                return { pipeline_layout_creation_result };
            }
        }
    };
}

#endif //GAME_PROJ_PIPELINE_LAYOUT_BUILDER_HPP
