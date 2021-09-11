//
// Created by marco.silipo on 31.08.2021.
//

#ifndef GAME_PROJ_VENGINE_HPP
#define GAME_PROJ_VENGINE_HPP

#include "event_source.hpp"
#include "ram_file.hpp"
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "allocated_buffer.hpp"
#include "allocated_image.hpp"


#include <glm/glm.hpp>
#include <vector>
#include <optional>
#include <entt/entt.hpp>

namespace vengine
{
    class vengine
    {

#pragma region GLFW
    private:
        void glfw_set_window_callbacks();

        void glfw_unset_window_callbacks();

        void *m_window_handle { };

        friend class vengine;

        void glfw_window_init(int width, int height, const std::string &title);

        void glfw_window_destroy();

    public:
        struct size
        {
            int width;
            int height;
        };

        struct camera_data
        {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 view_projection;


            void set_gpu_buffer_data(allocated_buffer buffer) const
            {
                void* data;
                vmaMapMemory(buffer.allocator, buffer.allocation, &data);
                memcpy(data, this, sizeof(camera_data));
                vmaUnmapMemory(buffer.allocator, buffer.allocation);
            }
        };

        struct frame_data
        {
            void bind_graphics_pipeline(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout,
                                        VkPipeline pipeline)
            {
                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(
                        command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline_layout,
                        0,
                        1,
                        &descriptor_set,
                        0,
                        nullptr);
            }

            VkSemaphore present_semaphore;
            VkSemaphore render_semaphore;
            VkFence render_fence;
            VkCommandPool command_pool;
            std::vector<VkCommandBuffer> command_buffers;

            allocated_buffer camera_buffer;
            VkDescriptorSet descriptor_set;
        };

        [[maybe_unused]] void window_size(int width, int height)
        {
            window_size({ width, height });
        }

        [[maybe_unused]] void window_size(const size &s);

        [[maybe_unused]] [[nodiscard]] size window_size() const;

        [[maybe_unused]] void window_title(const std::string &title);

        static void handle_pending_events();

        void swap_buffers();

    public:
        utils::event_source<vengine, utils::event_args> on_window_close;

        struct on_window_focused_event_args
        {
            [[maybe_unused]] bool has_focus;
        };
        utils::event_source<vengine, on_window_focused_event_args> on_window_focus;

        struct on_window_iconified_event_args
        {
            [[maybe_unused]] bool is_iconified;
        };
        utils::event_source<vengine, on_window_iconified_event_args> on_window_iconified;

        struct on_window_maximize_event_args
        {
            [[maybe_unused]] bool is_maximized;
        };
        utils::event_source<vengine, on_window_maximize_event_args> on_window_maximize;

        struct on_window_pos_event_args
        {
            [[maybe_unused]] int pos_x;
            [[maybe_unused]] int pos_y;
        };
        utils::event_source<vengine, on_window_pos_event_args> on_window_pos;

        utils::event_source<vengine, utils::event_args> on_window_refresh;

        struct on_window_size_event_args
        {
            [[maybe_unused]] int width;
            [[maybe_unused]] int height;
        };
        utils::event_source<vengine, on_window_size_event_args> on_window_size;

        struct on_window_content_scale_event_args
        {
            [[maybe_unused]] float scale_x;
            [[maybe_unused]] float scale_y;
        };
        utils::event_source<vengine, on_window_content_scale_event_args> on_window_content_scale;

        [[maybe_unused]] static void set_error_callback(void(*error_handle)(int error_code, const char *error_message));

#pragma endregion
    private:
        bool m_glfw_initialized;
        bool m_initialized;
        size_t m_frame_counter;
        size_t m_frame_data_index;

        vkb::Instance m_vkb_instance;
        VkSurfaceKHR m_vulkan_surface { };
        vkb::PhysicalDevice m_vkb_physical_device;
        vkb::Device m_vkb_device;
        vkb::Swapchain m_vkb_swap_chain;
        VkQueue m_vkb_graphics_queue;
        uint32_t m_vkb_graphics_queue_index;
        VkRenderPass m_vulkan_render_pass { };
        VmaAllocator m_vma_allocator;
        VkDescriptorPool m_descriptor_pool;
        VkDescriptorSetLayout m_descriptor_set_layout;
        std::vector<VkShaderModule> m_shader_modules;
        std::vector<VkImage> m_swap_chain_images;
        std::vector<VkImageView> m_swap_chain_image_views;
        std::vector<VkFramebuffer> m_frame_buffers;
        const size_t frame_data_structures_count = 2;
        std::vector<frame_data> m_frame_data_structures;

        VkFormat m_depths_format;
        allocated_image m_depth_image;
        VkImageView m_depths_image_view;
        entt::registry m_ecs;


        [[maybe_unused]] [[nodiscard]] std::optional<VkCommandBuffer> create_command_buffer(frame_data& frame);

        [[maybe_unused]] void destroy_command_buffer(frame_data& frame, VkCommandBuffer buffer);
    public:
        vengine();

        ~vengine();

        [[nodiscard]] size_t frame_count() const
        {
            return m_frame_counter;
        }

        [[nodiscard]] bool good() const
        {
            return m_glfw_initialized && m_initialized;
        }

        [[maybe_unused]] [[nodiscard]] std::optional<VkShaderModule> create_shader_module(const ram_file &file);

        [[maybe_unused]] void destroy_shader_module(VkShaderModule buffer);


        [[maybe_unused]] [[nodiscard]] VkDevice vulkan_device() const
        {
            return m_vkb_device.device;
        }

        [[maybe_unused]] [[nodiscard]] VkRenderPass vulkan_render_pass() const
        {
            return m_vulkan_render_pass;
        }

        [[maybe_unused]] [[nodiscard]] VmaAllocator allocator() const
        {
            return m_vma_allocator;
        }

        frame_data& current_frame_data() { return m_frame_data_structures[m_frame_data_index]; }

        [[maybe_unused]] [[nodiscard]] VkViewport vulkan_default_viewport() const
        {
            VkViewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) m_vkb_swap_chain.extent.width;
            viewport.height = (float) m_vkb_swap_chain.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            return viewport;
        }

        [[maybe_unused]] [[nodiscard]] VkRect2D vulkan_default_scissors() const
        {
            VkRect2D rect2d;
            rect2d.offset = {0,0};
            rect2d.extent = m_vkb_swap_chain.extent;
            return rect2d;
        }

        void render();

        [[nodiscard]] entt::registry& ecs() { return m_ecs; }

    public:
        struct on_render_pass_event_args
        {
            frame_data current_frame_data;
            VkCommandBuffer command_buffer;
        };
        using on_render_pass_event = utils::event_source<vengine, on_render_pass_event_args>;
        on_render_pass_event on_render_pass;
    };
}

#endif //GAME_PROJ_VENGINE_HPP
