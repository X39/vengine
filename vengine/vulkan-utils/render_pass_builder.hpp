//
// Created by marco.silipo on 08.09.2021.
//

#ifndef GAME_PROJ_RENDER_PASS_BUILDER_HPP
#define GAME_PROJ_RENDER_PASS_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "../allocated_image.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class render_pass_builder
    {
        VkDevice m_device;
        std::vector<VkAttachmentDescription> m_attachment_descriptions;
        std::vector<VkSubpassDescription> m_sub_pass_descriptions;
        std::vector<VkAttachmentReference> m_sub_pass_descriptions_attachment_references;
        std::vector<uint32_t> m_sub_pass_descriptions_preserve_attachments;
    public:
        explicit render_pass_builder(VkDevice device)
                : m_device(device)
        {

        }

        render_pass_builder& add_attachment_description(VkAttachmentDescription attachment_description)
        {
            m_attachment_descriptions.push_back(attachment_description);
            return *this;
        }
        render_pass_builder& add_attachment_description(
                VkAttachmentDescriptionFlags    flags,
                VkFormat                        format,
                VkSampleCountFlagBits           samples,
                VkAttachmentLoadOp              load_op,
                VkAttachmentStoreOp             store_op,
                VkAttachmentLoadOp              stencil_load_op,
                VkAttachmentStoreOp             stencil_store_op,
                VkImageLayout                   initial_layout,
                VkImageLayout                   final_layout)
        {
            VkAttachmentDescription attachment_description = { };
            attachment_description.flags = flags;
            attachment_description.format = format;
            attachment_description.samples = samples;
            attachment_description.loadOp = load_op;
            attachment_description.storeOp = store_op;
            attachment_description.stencilLoadOp = stencil_load_op;
            attachment_description.stencilStoreOp = stencil_store_op;
            attachment_description.initialLayout = initial_layout;
            attachment_description.finalLayout = final_layout;
            m_attachment_descriptions.push_back(attachment_description);
            return *this;
        }
        render_pass_builder& add_sub_pass_description(VkSubpassDescription sub_pass_description)
        {
            m_sub_pass_descriptions.push_back(sub_pass_description);
            return *this;
        }

        result<VkRenderPass> build() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_attachment_descriptions.size() > UINT32_MAX)
            {
                auto message = "More attachment descriptions have been added then vulkan can handle.";
                log::error("vengine::vulkan_utils::render_pass_builder::build()", message);
                return message;
            }
            if (m_sub_pass_descriptions.size() > UINT32_MAX)
            {
                auto message = "More sub pass descriptions have been added then vulkan can handle.";
                log::error("vengine::vulkan_utils::render_pass_builder::build()", message);
                return message;
            }
            
            VkRenderPassCreateInfo render_pass_info = { };
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = (uint32_t)m_attachment_descriptions.size();
            render_pass_info.pAttachments = m_attachment_descriptions.data();
            render_pass_info.subpassCount = (uint32_t)m_sub_pass_descriptions.size();
            render_pass_info.pSubpasses = m_sub_pass_descriptions.data();


            VkRenderPass result = {};
            auto create_render_pass_result = vkCreateRenderPass(
                    m_device,
                    &render_pass_info,
                    nullptr,
                    &result);
            if (create_render_pass_result == VK_SUCCESS)
            {
                return { result };
            }
            else
            {
                auto message = std::string("Failed to build render pass (").append(stringify::data(create_render_pass_result)).append(").");
                log::error("vengine::vulkan_utils::render_pass_builder::build()", message);
                return { create_render_pass_result, message };
            }
        }
    };
}

#endif //GAME_PROJ_RENDER_PASS_BUILDER_HPP
