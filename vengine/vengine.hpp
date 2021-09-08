//
// Created by marco.silipo on 31.08.2021.
//

#ifndef GAME_PROJ_VENGINE_HPP
#define GAME_PROJ_VENGINE_HPP

#include "event_source.hpp"
#include "ram_file.hpp"
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "allocated_image.hpp"

#include <vector>
#include <optional>

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

        vkb::Instance m_vkb_instance;
        VkSurfaceKHR m_vulkan_surface { };
        vkb::PhysicalDevice m_vkb_physical_device;
        vkb::Device m_vkb_device;
        vkb::Swapchain m_vkb_swap_chain;
        VkQueue m_vkb_graphics_queue;
        uint32_t m_vkb_graphics_queue_index;
        VkCommandPool m_vulkan_command_pool { };
        VkRenderPass m_vulkan_render_pass { };
        VkSemaphore m_vulkan_present_semaphore { };
        VkSemaphore m_vulkan_render_semaphore { };
        VkFence m_vulkan_render_fence { };
        VmaAllocator m_vma_allocator;
        std::vector<VkShaderModule> m_shader_modules;
        std::vector<VkCommandBuffer> m_command_buffers;
        std::vector<VkImage> m_swap_chain_images;
        std::vector<VkImageView> m_swap_chain_image_views;
        std::vector<VkFramebuffer> m_frame_buffers;

        VkFormat m_depths_format;
        allocated_image m_depth_image;
        VkImageView m_depths_image_view;


        std::vector<std::string> m_errors;
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

        [[nodiscard]] std::vector<std::string> get_errors() const
        {
            return m_errors;
        }

        [[maybe_unused]] void clear_errors()
        {
            m_errors.clear();
        }

        [[maybe_unused]] [[nodiscard]] std::optional<VkCommandBuffer> create_command_buffer();

        [[maybe_unused]] void destroy_command_buffer(VkCommandBuffer buffer);

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

    public:
        utils::event_source<vengine, utils::event_args> on_render_pass;
    };
}

#endif //GAME_PROJ_VENGINE_HPP
