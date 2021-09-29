//
// Created by marco.silipo on 30.09.2021.
//

#ifndef GAME_PROJ_SUBMIT_BUILDER_HPP
#define GAME_PROJ_SUBMIT_BUILDER_HPP
#include "result.hpp"
#include "../log.hpp"
#include "../allocated_image.hpp"
#include "stringify.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class submit_builder
    {
        struct wait_tuple
        {
            VkSemaphore semaphore;
            VkPipelineStageFlags pipeline_stage_flags;
        };
        VkQueue m_queue;
        VkFence m_fence;
        std::vector<wait_tuple> m_wait_tuples;
        std::vector<VkSemaphore> m_signal_semaphores;
        std::vector<VkCommandBuffer> m_command_buffers;
    public:
        explicit submit_builder(VkQueue queue, VkFence fence)
                : m_queue(queue), m_fence(fence)
        {

        }
        submit_builder& add_wait_semaphore(VkSemaphore semaphore, VkPipelineStageFlags pipeline_stage_flags)
        {
            m_wait_tuples.push_back({ semaphore, pipeline_stage_flags });
            return *this;
        }
        submit_builder& add_signal_semaphore(VkSemaphore semaphore)
        {
            m_signal_semaphores.push_back(semaphore);
            return *this;
        }
        submit_builder& add_command_buffer(VkCommandBuffer command_buffer)
        {
            m_command_buffers.push_back(command_buffer);
            return *this;
        }
        template<typename iterator>
        submit_builder& add_command_buffer(iterator begin, iterator end)
        {
            m_command_buffers.insert(m_command_buffers.end(), begin, end);
            return *this;
        }

        result<void> submit() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_signal_semaphores.size() > UINT32_MAX)
            {
                auto message = "More signal semaphores have been added then vulkan can handle.";
                log::error("vengine::vulkan_utils::submit_builder::submit()", message);
                return message;
            }
            if (m_wait_tuples.size() > UINT32_MAX)
            {
                auto message = "More semaphore pairs have been added then vulkan can handle.";
                log::error("vengine::vulkan_utils::submit_builder::submit()", message);
                return message;
            }
            if (m_command_buffers.size() > UINT32_MAX)
            {
                auto message = "More command buffers have been added then vulkan can handle.";
                log::error("vengine::vulkan_utils::submit_builder::submit()", message);
                return message;
            }

            std::vector<VkPipelineStageFlags> pipeline_stage_flags(m_wait_tuples.size());
            std::vector<VkSemaphore> semaphores(m_wait_tuples.size());
            for (size_t i = 0; i < m_wait_tuples.size(); i++)
            {
                semaphores[i] = m_wait_tuples[i].semaphore;
                pipeline_stage_flags[i] = m_wait_tuples[i].pipeline_stage_flags;
            }

            VkSubmitInfo submit_info = { };
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = nullptr;

            submit_info.waitSemaphoreCount = (uint32_t)m_wait_tuples.size();
            submit_info.pWaitSemaphores = semaphores.data();
            submit_info.pWaitDstStageMask = pipeline_stage_flags.data();

            submit_info.commandBufferCount = (uint32_t)m_command_buffers.size();
            submit_info.pCommandBuffers = m_command_buffers.data();

            submit_info.signalSemaphoreCount = (uint32_t)m_signal_semaphores.size();
            submit_info.pSignalSemaphores = m_signal_semaphores.data();

            auto queue_submit_result = vkQueueSubmit(m_queue, 1, &submit_info, m_fence);
            if (queue_submit_result != VK_SUCCESS)
            {
                auto message = std::string("Failed submit to queue (").append(stringify::data(queue_submit_result)).append(")");
                log::error("vengine::vulkan_utils::submit_builder::submit()", message);
                return { queue_submit_result, message };
            }
            return { };
        }
    };
}

#endif //GAME_PROJ_SUBMIT_BUILDER_HPP
