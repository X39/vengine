//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_PIPELINE_BUILDER_HPP
#define GAME_PROJ_PIPELINE_BUILDER_HPP

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class pipeline_builder
    {
        VkDevice m_device;
        VkRenderPass m_render_pass;
        std::vector<VkPipelineShaderStageCreateInfo> m_shader_stage_create_infos;
        VkPipelineVertexInputStateCreateInfo m_vertex_input_state_create_info;
        VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state_create_info;
        VkPipelineRasterizationStateCreateInfo m_rasterization_state_create_info;
        VkPipelineMultisampleStateCreateInfo m_multisample_state_create_info;
        std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;
        VkPipelineLayoutCreateInfo m_layout_create_info;
    public:
        pipeline_builder(VkDevice device, VkRenderPass render_pass)
                : m_device(device), m_render_pass(render_pass), m_vertex_input_state_create_info({ }),
                  m_input_assembly_state_create_info({ }), m_rasterization_state_create_info({ }),
                  m_multisample_state_create_info({ })
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

        pipeline_builder &set_vertex_input()
        {
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = { };
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.pNext = nullptr;

            // Unused as of now so this can be left at 0.
            vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
            vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
            m_vertex_input_state_create_info = vertex_input_state_create_info;
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

        pipeline_builder &set_layout()
        {
            VkPipelineLayoutCreateInfo layout_create_info{};
            layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layout_create_info.pNext = nullptr;
            layout_create_info.flags = 0;
            layout_create_info.setLayoutCount = 0;
            layout_create_info.pSetLayouts = nullptr;
            layout_create_info.pushConstantRangeCount = 0;
            layout_create_info.pPushConstantRanges = nullptr;

            m_layout_create_info = layout_create_info;

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

        void build(VkViewport viewport, VkRect2D scissors)
        {

            // Viewport State
            VkPipelineViewportStateCreateInfo viewport_state_create_info = { };
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.pNext = nullptr;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &scissors;

            // Color Blend State
            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = { };
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.pNext = nullptr;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount = m_color_blend_attachment_states.size();
            color_blend_state_create_info.pAttachments = m_color_blend_attachment_states.data();


            // Pipeline
            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.pNext = nullptr;
            pipelineInfo.stageCount = m_shader_stage_create_infos.size();
            pipelineInfo.pStages = m_shader_stage_create_infos.data();
            pipelineInfo.pVertexInputState = &m_vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &m_input_assembly_state_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &m_rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &m_multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.layout = m_layout_create_info;
            pipelineInfo.renderPass = m_render_pass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
            VkPipeline newPipeline;
            if (vkCreateGraphicsPipelines(
                    device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
                std::cout << "failed to create pipeline\n";
                return VK_NULL_HANDLE; // failed to create graphics pipeline
            }
            else
            {
                return newPipeline;
            }
        }
    };
}

#endif //GAME_PROJ_PIPELINE_BUILDER_HPP
