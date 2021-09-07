//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_PIPELINE_BUILDER_HPP
#define GAME_PROJ_PIPELINE_BUILDER_HPP

#include "result.hpp"
#include "../log.hpp"
#include "stringify.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class pipeline_builder
    {
        VkDevice m_device;
        VkRenderPass m_render_pass;
        VkViewport m_viewport;
        VkRect2D m_scissors;
        VkPipelineLayout m_pipeline_layout;
        std::vector<VkPipelineShaderStageCreateInfo> m_shader_stage_create_infos;
        std::optional<VkPipelineInputAssemblyStateCreateInfo> m_input_assembly_state_create_info;
        std::optional<VkPipelineRasterizationStateCreateInfo> m_rasterization_state_create_info;
        std::optional<VkPipelineMultisampleStateCreateInfo> m_multisample_state_create_info;
        std::optional<VkPipelineLayoutCreateInfo> m_layout_create_info;
        std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;
        std::vector<VkVertexInputBindingDescription> m_vertex_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> m_vertex_input_attribute_descriptions;
    public:
        pipeline_builder(VkDevice device, VkRenderPass render_pass, VkViewport viewport, VkRect2D scissors, VkPipelineLayout pipeline_layout)
                : m_device(device), m_render_pass(render_pass), m_viewport(viewport), m_scissors(scissors), m_pipeline_layout(pipeline_layout)
        {

        }

        pipeline_builder &add_shader(VkShaderModule shader_module, VkShaderStageFlagBits stage_flag_bits,
                                     const char *entry_method = "main")
        {
            VkPipelineShaderStageCreateInfo shader_stage_create_info { };
            shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info.pNext = nullptr;

            shader_stage_create_info.stage = stage_flag_bits;
            shader_stage_create_info.module = shader_module;
            shader_stage_create_info.pName = entry_method;
            m_shader_stage_create_infos.push_back(shader_stage_create_info);
            return *this;
        }


        pipeline_builder &add_vertex_input_binding_descriptions(std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions)
        {
            for(auto& vertex_input_binding_description : vertex_input_binding_descriptions)
            {
                m_vertex_input_binding_descriptions.push_back(vertex_input_binding_description);
            }
            return *this;
        }
        pipeline_builder &add_vertex_input_attribute_descriptions(std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions)
        {
            for(auto& vertex_input_attribute_description : vertex_input_attribute_descriptions)
            {
                m_vertex_input_attribute_descriptions.push_back(vertex_input_attribute_description);
            }
            return *this;
        }
        pipeline_builder &add_vertex_input_binding_description(VkVertexInputBindingDescription vertex_input_binding_description)
        {
            m_vertex_input_binding_descriptions.push_back(vertex_input_binding_description);
            return *this;
        }
        pipeline_builder &add_vertex_input_attribute_description(VkVertexInputAttributeDescription vertex_input_attribute_description)
        {
            m_vertex_input_attribute_descriptions.push_back(vertex_input_attribute_description);
            return *this;
        }

        pipeline_builder &set_input_assembly(VkPrimitiveTopology primitive_topology)
        {
            VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = { };
            input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_state_create_info.pNext = nullptr;

            input_assembly_state_create_info.topology = primitive_topology;
            input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
            m_input_assembly_state_create_info = input_assembly_state_create_info;
            return *this;
        }

        pipeline_builder &set_rasterization(VkPolygonMode polygon_mode)
        {
            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = { };
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.pNext = nullptr;

            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

            rasterization_state_create_info.polygonMode = polygon_mode;
            rasterization_state_create_info.lineWidth = 1.0f;

            // Backface Culling
            rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

            // Depth Bias
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            m_rasterization_state_create_info = rasterization_state_create_info;
            return *this;
        }

        pipeline_builder &set_multisample()
        {
            VkPipelineMultisampleStateCreateInfo multisample_state_create_info = { };
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.pNext = nullptr;

            multisample_state_create_info.sampleShadingEnable = VK_FALSE;

            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisample_state_create_info.minSampleShading = 1.0f;
            multisample_state_create_info.pSampleMask = nullptr;
            multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
            multisample_state_create_info.alphaToOneEnable = VK_FALSE;

            m_multisample_state_create_info = multisample_state_create_info;

            return *this;
        }

        pipeline_builder &add_color_blend()
        {
            VkPipelineColorBlendAttachmentState color_blend_attachment_state = { };
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                                          | VK_COLOR_COMPONENT_G_BIT
                                                          | VK_COLOR_COMPONENT_B_BIT
                                                          | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment_state.blendEnable = VK_FALSE;

            m_color_blend_attachment_states.push_back(color_blend_attachment_state);

            return *this;
        }

        result<VkPipeline> build()
        {
            if (!m_input_assembly_state_create_info.has_value())
            {
                auto message = "Input-Assembly-State not set. (set_input_assembly)";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (!m_rasterization_state_create_info.has_value())
            {
                auto message = "Rasterization-State not set. (set_rasterization)";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (!m_multisample_state_create_info.has_value())
            {
                auto message = "Multisample-State not set. (set_multisample)";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (m_shader_stage_create_infos.empty())
            {
                auto message = "No shaders are present. (add_shader)";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (m_shader_stage_create_infos.size() > UINT32_MAX)
            {
                auto message = "More shaders have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (m_color_blend_attachment_states.size() > UINT32_MAX)
            {
                auto message = "More color blending modes have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (m_vertex_input_attribute_descriptions.size() > UINT32_MAX)
            {
                auto message = "More color input attribute descriptions have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }
            if (m_vertex_input_binding_descriptions.size() > UINT32_MAX)
            {
                auto message = "More color vertex input binding descriptions have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return message;
            }

            // Vertex Input State
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = { };
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.pNext = nullptr;

            vertex_input_state_create_info.pVertexBindingDescriptions = m_vertex_input_binding_descriptions.data();
            vertex_input_state_create_info.vertexBindingDescriptionCount = (uint32_t)m_vertex_input_binding_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions = m_vertex_input_attribute_descriptions.data();
            vertex_input_state_create_info.vertexAttributeDescriptionCount = (uint32_t)m_vertex_input_attribute_descriptions.size();

            // Viewport State
            VkPipelineViewportStateCreateInfo viewport_state_create_info = { };
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.pNext = nullptr;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_scissors;

            // Color Blend State
            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = { };
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.pNext = nullptr;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount = (uint32_t)m_color_blend_attachment_states.size();
            color_blend_state_create_info.pAttachments = m_color_blend_attachment_states.data();


            // Pipeline
            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.pNext = nullptr;
            pipelineInfo.stageCount = (uint32_t)m_shader_stage_create_infos.size();
            pipelineInfo.pStages = m_shader_stage_create_infos.data();
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &m_input_assembly_state_create_info.value();
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &m_rasterization_state_create_info.value();
            pipelineInfo.pMultisampleState = &m_multisample_state_create_info.value();
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.layout = m_pipeline_layout;
            pipelineInfo.renderPass = m_render_pass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
            VkPipeline pipeline;
            auto pipeline_creation_result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
            if (pipeline_creation_result == VK_SUCCESS)
            {
                return { pipeline };
            }
            else
            {
                auto message = std::string("Failed to build pipeline (").append(stringify::data(pipeline_creation_result)).append(")");
                log::error("vengine::vulkan_utils::pipeline_builder::build()", message);
                return { pipeline_creation_result };
            }
        }
    };
}

#endif //GAME_PROJ_PIPELINE_BUILDER_HPP
