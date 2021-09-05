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
    public:
        explicit pipeline_layout_builder(VkDevice device)
                : m_device(device)
        {

        }

        result<VkPipelineLayout> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;

            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = 0;
            pipeline_layout_create_info.pSetLayouts = nullptr;
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;

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
