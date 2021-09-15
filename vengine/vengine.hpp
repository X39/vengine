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
#include "vulkan-utils/result.hpp"


#include <glm/glm.hpp>
#include <vector>
#include <optional>

namespace vengine
{
    class vengine
    {
    public:
        struct size
        {
            int width;
            int height;
        };

        struct gpu_camera_data
        {
            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 view_projection;


            void set_gpu_buffer_data(allocated_buffer buffer) const
            {
                void* data;
                vmaMapMemory(buffer.allocator, buffer.allocation, &data);
                memcpy(data, this, sizeof(gpu_camera_data));
                vmaUnmapMemory(buffer.allocator, buffer.allocation);
            }
        };
        struct gpu_mesh_data
        {
            glm::mat4 matrix;
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


            const size_t mesh_buffer_size = 1000;
            allocated_buffer camera_buffer;
            allocated_buffer mesh_buffer;
            VkDescriptorSet descriptor_set;
        };

#pragma region GLFW
    private:
        void glfw_set_window_callbacks();

        void glfw_unset_window_callbacks();

        void *m_window_handle { };

        friend class vengine;

        void glfw_window_init(int width, int height, const std::string &title);

        void glfw_window_destroy();

    public:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        enum class mouse_buttons
        {
            MOUSE_BUTTON_1 = 0,
            MOUSE_BUTTON_2 = 1,
            MOUSE_BUTTON_3 = 2,
            MOUSE_BUTTON_4 = 3,
            MOUSE_BUTTON_5 = 4,
            MOUSE_BUTTON_6 = 5,
            MOUSE_BUTTON_7 = 6,
            MOUSE_BUTTON_8 = 7,
            MOUSE_BUTTON_LAST = MOUSE_BUTTON_8,
            MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
            MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
            MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3,
        };
        enum class key_actions
        {
            /*! @name Key and button actions
             *  @{ */
            /*! @brief The key or mouse button was released.
             *
             *  The key or mouse button was released.
             *
             *  @ingroup input
             */
            RELEASE = 0,
            /*! @brief The key or mouse button was pressed.
             *
             *  The key or mouse button was pressed.
             *
             *  @ingroup input
             */
            PRESS = 1,
            /*! @brief The key was held down until it repeated.
             *
             *  The key was held down until it repeated.
             *
             *  @ingroup input
             */
            REPEAT = 2,
        };
        enum class keys
        {
            /* The unknown key */
            KEY_UNKNOWN = -1,

            /* Printable keys */
            KEY_SPACE = 32,
            KEY_APOSTROPHE = 39  /* ' */,
            KEY_COMMA = 44  /* , */,
            KEY_MINUS = 45  /* - */,
            KEY_PERIOD = 46  /* . */,
            KEY_SLASH = 47  /* / */,
            KEY_0 = 48,
            KEY_1 = 49,
            KEY_2 = 50,
            KEY_3 = 51,
            KEY_4 = 52,
            KEY_5 = 53,
            KEY_6 = 54,
            KEY_7 = 55,
            KEY_8 = 56,
            KEY_9 = 57,
            KEY_SEMICOLON = 59  /* ; */,
            KEY_EQUAL = 61  /* = */,
            KEY_A = 65,
            KEY_B = 66,
            KEY_C = 67,
            KEY_D = 68,
            KEY_E = 69,
            KEY_F = 70,
            KEY_G = 71,
            KEY_H = 72,
            KEY_I = 73,
            KEY_J = 74,
            KEY_K = 75,
            KEY_L = 76,
            KEY_M = 77,
            KEY_N = 78,
            KEY_O = 79,
            KEY_P = 80,
            KEY_Q = 81,
            KEY_R = 82,
            KEY_S = 83,
            KEY_T = 84,
            KEY_U = 85,
            KEY_V = 86,
            KEY_W = 87,
            KEY_X = 88,
            KEY_Y = 89,
            KEY_Z = 90,
            KEY_LEFT_BRACKET = 91  /* [ */,
            KEY_BACKSLASH = 92  /* \ */,
            KEY_RIGHT_BRACKET = 93  /* ] */,
            KEY_GRAVE_ACCENT = 96  /* ` */,
            KEY_WORLD_1 = 161 /* non-US #1 */,
            KEY_WORLD_2 = 162 /* non-US #2 */,

            /* Function keys */
            KEY_ESCAPE = 256,
            KEY_ENTER = 257,
            KEY_TAB = 258,
            KEY_BACKSPACE = 259,
            KEY_INSERT = 260,
            KEY_DELETE = 261,
            KEY_RIGHT = 262,
            KEY_LEFT = 263,
            KEY_DOWN = 264,
            KEY_UP = 265,
            KEY_PAGE_UP = 266,
            KEY_PAGE_DOWN = 267,
            KEY_HOME = 268,
            KEY_END = 269,
            KEY_CAPS_LOCK = 280,
            KEY_SCROLL_LOCK = 281,
            KEY_NUM_LOCK = 282,
            KEY_PRINT_SCREEN = 283,
            KEY_PAUSE = 284,
            KEY_F1 = 290,
            KEY_F2 = 291,
            KEY_F3 = 292,
            KEY_F4 = 293,
            KEY_F5 = 294,
            KEY_F6 = 295,
            KEY_F7 = 296,
            KEY_F8 = 297,
            KEY_F9 = 298,
            KEY_F10 = 299,
            KEY_F11 = 300,
            KEY_F12 = 301,
            KEY_F13 = 302,
            KEY_F14 = 303,
            KEY_F15 = 304,
            KEY_F16 = 305,
            KEY_F17 = 306,
            KEY_F18 = 307,
            KEY_F19 = 308,
            KEY_F20 = 309,
            KEY_F21 = 310,
            KEY_F22 = 311,
            KEY_F23 = 312,
            KEY_F24 = 313,
            KEY_F25 = 314,
            KEY_KP_0 = 320,
            KEY_KP_1 = 321,
            KEY_KP_2 = 322,
            KEY_KP_3 = 323,
            KEY_KP_4 = 324,
            KEY_KP_5 = 325,
            KEY_KP_6 = 326,
            KEY_KP_7 = 327,
            KEY_KP_8 = 328,
            KEY_KP_9 = 329,
            KEY_KP_DECIMAL = 330,
            KEY_KP_DIVIDE = 331,
            KEY_KP_MULTIPLY = 332,
            KEY_KP_SUBTRACT = 333,
            KEY_KP_ADD = 334,
            KEY_KP_ENTER = 335,
            KEY_KP_EQUAL = 336,
            KEY_LEFT_SHIFT = 340,
            KEY_LEFT_CONTROL = 341,
            KEY_LEFT_ALT = 342,
            KEY_LEFT_SUPER = 343,
            KEY_RIGHT_SHIFT = 344,
            KEY_RIGHT_CONTROL = 345,
            KEY_RIGHT_ALT = 346,
            KEY_RIGHT_SUPER = 347,
            KEY_MENU = 348,

            KEY_LAST = KEY_MENU,
        };
        enum class key_mods
        {
            /*! @brief If this bit is set one or more Shift keys were held down.
             *
             *  If this bit is set one or more Shift keys were held down.
             */
            MOD_SHIFT = 0x0001,
            /*! @brief If this bit is set one or more Control keys were held down.
             *
             *  If this bit is set one or more Control keys were held down.
             */
            MOD_CONTROL = 0x0002,
            /*! @brief If this bit is set one or more Alt keys were held down.
             *
             *  If this bit is set one or more Alt keys were held down.
             */
            MOD_ALT = 0x0004,
            /*! @brief If this bit is set one or more Super keys were held down.
             *
             *  If this bit is set one or more Super keys were held down.
             */
            MOD_SUPER = 0x0008,
            /*! @brief If this bit is set the Caps Lock key is enabled.
             *
             *  If this bit is set the Caps Lock key is enabled and the @ref
             *  LOCK_KEY_MODS input mode is set.
             */
            MOD_CAPS_LOCK = 0x0010,
            /*! @brief If this bit is set the Num Lock key is enabled.
             *
             *  If this bit is set the Num Lock key is enabled and the @ref
             *  LOCK_KEY_MODS input mode is set.
             */
            MOD_NUM_LOCK = 0x0020,
        };
#pragma clang diagnostic pop

        [[maybe_unused]] void window_size(int width, int height)
        {
            window_size({ width, height });
        }

        [[maybe_unused]] void window_size(const size &s);

        [[maybe_unused]] [[nodiscard]] size window_size() const;

        [[maybe_unused]] [[maybe_unused]] void window_title(const std::string &title);

        static void handle_pending_events();

        void swap_buffers();

    public:

#pragma region events
        struct on_key_event_args
        {
            [[maybe_unused]] keys key;
            [[maybe_unused]] key_mods mods;
            [[maybe_unused]] key_actions action;
            [[maybe_unused]] int scancode;
        };
        using on_key_event = utils::event_source<vengine, on_key_event_args>;
        on_key_event on_key;

        struct on_text_event_args
        {
            [[maybe_unused]] unsigned int unicode_codepoint;
        };
        using on_text_event = utils::event_source<vengine, on_text_event_args>;
        on_text_event on_text;

        double on_mouse_move_old_x_pos{};
        double on_mouse_move_old_y_pos{};
        struct on_mouse_move_event_args
        {
            [[maybe_unused]] double pos_x;
            [[maybe_unused]] double pos_y;
            [[maybe_unused]] double delta_x;
            [[maybe_unused]] double delta_y;
        };
        using on_mouse_move_event = utils::event_source<vengine, on_mouse_move_event_args>;
        on_mouse_move_event on_mouse_move;

        struct on_mouse_scroll_event_args
        {
            [[maybe_unused]] double offset_x;
            [[maybe_unused]] double offset_y;
        };
        using on_mouse_scroll_event = utils::event_source<vengine, on_mouse_scroll_event_args>;
        on_mouse_scroll_event on_mouse_scroll;

        struct on_mouse_button_event_args
        {
            [[maybe_unused]] mouse_buttons pos_x;
            [[maybe_unused]] key_mods mods;
            [[maybe_unused]] key_actions action;
        };
        using on_mouse_button_event = utils::event_source<vengine, on_mouse_button_event_args>;
        on_mouse_button_event on_mouse_button;


        using on_window_close_event = utils::event_source<vengine, utils::event_args>;
        on_window_close_event on_window_close;

        struct on_window_focused_event_args
        {
            [[maybe_unused]] bool has_focus;
        };
        using on_window_focus_event = utils::event_source<vengine, on_window_focused_event_args>;
        on_window_focus_event on_window_focus;

        struct on_window_iconified_event_args
        {
            [[maybe_unused]] bool is_iconified;
        };
        using on_window_iconified_event = utils::event_source<vengine, on_window_iconified_event_args>;
        on_window_iconified_event on_window_iconified;

        struct on_window_maximize_event_args
        {
            [[maybe_unused]] bool is_maximized;
        };
        using on_window_maximize_event = utils::event_source<vengine, on_window_maximize_event_args>;
        on_window_maximize_event on_window_maximize;

        struct on_window_pos_event_args
        {
            [[maybe_unused]] int pos_x;
            [[maybe_unused]] int pos_y;
        };
        utils::event_source<vengine, on_window_pos_event_args> on_window_pos;

        using on_window_refresh_event = utils::event_source<vengine, utils::event_args>;
        on_window_refresh_event on_window_refresh;

        struct on_window_size_event_args
        {
            [[maybe_unused]] int width;
            [[maybe_unused]] int height;
        };
        using on_window_size_event = utils::event_source<vengine, on_window_size_event_args>;
        on_window_size_event on_window_size;

        struct on_window_content_scale_event_args
        {
            [[maybe_unused]] float scale_x;
            [[maybe_unused]] float scale_y;
        };
        using on_window_content_scale_event = utils::event_source<vengine, on_window_content_scale_event_args>;
        on_window_content_scale_event on_window_content_scale;
#pragma endregion
#pragma region on_key...

#pragma endregion

        [[maybe_unused]] static void set_error_callback(void(*error_handle)(int error_code, const char *error_message));

#pragma endregion
    private:
        bool m_glfw_initialized{};
        bool m_initialized{};
        size_t m_frame_counter{};
        size_t m_frame_data_index{};

        vkb::Instance m_vkb_instance{};
        VkSurfaceKHR m_vulkan_surface { };
        vkb::PhysicalDevice m_vkb_physical_device{};
        vkb::Device m_vkb_device{};
        vkb::Swapchain m_vkb_swap_chain{};
        VkQueue m_vkb_graphics_queue{};
        uint32_t m_vkb_graphics_queue_index{};
        VkRenderPass m_vulkan_render_pass{};
        VmaAllocator m_vma_allocator{};
        VkDescriptorPool m_descriptor_pool{};
        VkDescriptorSetLayout m_descriptor_set_layout{};
        VkPhysicalDeviceProperties m_physical_device_properties{};
        std::vector<VkShaderModule> m_shader_modules{};
        std::vector<VkImage> m_swap_chain_images{};
        std::vector<VkImageView> m_swap_chain_image_views{};
        std::vector<VkFramebuffer> m_frame_buffers{};
        const size_t frame_data_structures_count = 2;
        std::vector<frame_data> m_frame_data_structures{};

        VkFormat m_depths_format{};
        allocated_image m_depth_image{};
        VkImageView m_depths_image_view{};


        [[maybe_unused]] [[nodiscard]] std::optional<VkCommandBuffer> create_command_buffer(frame_data& frame) const;

        [[maybe_unused]] [[maybe_unused]] void destroy_command_buffer(frame_data& frame, VkCommandBuffer buffer) const;
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
        [[maybe_unused]] [[nodiscard]] VkDescriptorSetLayout vulkan_descriptor_set_layout() const
        {
            return m_descriptor_set_layout;
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

        vulkan_utils::result<void> render();

        [[nodiscard]] const VkPhysicalDeviceProperties& physical_device_properties() const { return m_physical_device_properties; }

        [[nodiscard]] size_t gpu_pad(size_t original_size) const
        {
            size_t padding = physical_device_properties().limits.minUniformBufferOffsetAlignment;
            size_t mod_size = original_size % padding;
            size_t align_missing = padding - mod_size;
            return original_size + align_missing;
        }

    public:
        struct on_render_pass_event_args
        {
            frame_data current_frame_data;
            VkCommandBuffer command_buffer{};
        };
        using on_render_pass_event = utils::event_source<vengine, on_render_pass_event_args>;
        on_render_pass_event on_render_pass;
    };
}

#endif //GAME_PROJ_VENGINE_HPP
