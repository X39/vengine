//
// Created by marco.silipo on 31.08.2021.
//

#ifndef GAME_PROJ_VENGINE_H
#define GAME_PROJ_VENGINE_H

#include "event_source.hpp"
#include "VkBootstrap.h"

namespace vengine
{
    class vengine
    {
    public:

        class glfw_window
        {
            void glfw_set_window_callbacks();

            void glfw_unset_window_callbacks();
            void* m_window_handle;
            bool m_initialized;
            friend class vengine;
            glfw_window(int width, int height, const std::string& title);
            ~glfw_window();

        public:
            struct size { int width; int height; };
            [[maybe_unused]] void window_size(int width, int height) { window_size({width, height}); }
            [[maybe_unused]] void window_size(const size& s);
            [[maybe_unused]] [[nodiscard]] size window_size() const;

            [[maybe_unused]] void window_title(const std::string& title);
            static void handle_pending_events();
            void swap_buffers();
        public:
            utils::event_source<glfw_window, utils::event_args> on_window_close;

            struct on_window_focused_event_args
            {
                [[maybe_unused]] bool has_focus;
            };
            utils::event_source<glfw_window, on_window_focused_event_args> on_window_focus;

            struct on_window_iconified_event_args
            {
                [[maybe_unused]] bool is_iconified;
            };
            utils::event_source<glfw_window, on_window_iconified_event_args> on_window_iconified;

            struct on_window_maximize_event_args
            {
                [[maybe_unused]] bool is_maximized;
            };
            utils::event_source<glfw_window, on_window_maximize_event_args> on_window_maximize;

            struct on_window_pos_event_args
            {
                [[maybe_unused]] int pos_x;
                [[maybe_unused]] int pos_y;
            };
            utils::event_source<glfw_window, on_window_pos_event_args> on_window_pos;

            utils::event_source<glfw_window, utils::event_args> on_window_refresh;

            struct on_window_size_event_args
            {
                [[maybe_unused]] int width;
                [[maybe_unused]] int height;
            };
            utils::event_source<glfw_window, on_window_size_event_args> on_window_size;

            struct on_window_content_scale_event_args
            {
                [[maybe_unused]] float scale_x;
                [[maybe_unused]] float scale_y;
            };
            utils::event_source<glfw_window, on_window_content_scale_event_args> on_window_content_scale;

            [[maybe_unused]] static void set_error_callback(void(*error_handle)(int error_code, const char* error_message));
        };
    private:
        glfw_window m_glfw;
        bool m_initialized;
        vkb::Instance m_vulkan_instance;
        VkSurfaceKHR m_vulkan_surface{};
        vkb::PhysicalDevice m_vulkan_physical_device;
        vkb::Device m_vulkan_device;
        vkb::Swapchain m_vulkan_swap_chain;
        VkQueue m_vulkan_graphics_queue;
    public:
        vengine();
        ~vengine();

        [[maybe_unused]] [[nodiscard]] glfw_window& window() { return m_glfw; }
        [[maybe_unused]] [[nodiscard]] const glfw_window& window() const { return m_glfw; }

        [[nodiscard]] bool good() const { return m_glfw.m_initialized && m_initialized; }

    };
}

#endif //GAME_PROJ_VENGINE_H
