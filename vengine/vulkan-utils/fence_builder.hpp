//
// Created by marco.silipo on 30.09.2021.
//

#ifndef GAME_PROJ_FENCE_BUILDER_HPP
#define GAME_PROJ_FENCE_BUILDER_HPP
#include "result.hpp"
#include "../log.hpp"
#include "../allocated_image.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class fence_builder
    {
        VkDevice m_device;
        std::optional<VkFenceCreateFlags> m_fence_create_flags;
    public:
        explicit fence_builder(VkDevice device)
                : m_device(device)
        {

        }
        fence_builder& set_fence_create_flags(VkFenceCreateFlags fence_create_flags)
        {
            m_fence_create_flags = fence_create_flags;
            return *this;
        }

        result<VkFence> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (!m_fence_create_flags.has_value())
            {
                auto message = "fence create flags never have been set. (set_fence_create_flags)";
                log::error("vengine::vulkan_utils::fence_builder::build()", message);
                return message;
            }

            VkFenceCreateInfo fence_create_info = { };
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.pNext = nullptr;

            fence_create_info.flags = m_fence_create_flags.value();

            VkFence fence;
            auto fence_create_result = vkCreateFence(m_device, &fence_create_info, nullptr, &fence);
            if (fence_create_result != VK_SUCCESS)
            {
                auto message = std::string("Failed submit to queue (").append(stringify::data(fence_create_result)).append(")");
                log::error("vengine::vulkan_utils::fence_builder::build()", message);
                return { fence_create_result, message };
            }
            return { fence };
        }
    };
}

#endif //GAME_PROJ_FENCE_BUILDER_HPP
