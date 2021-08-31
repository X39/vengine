//
// Created by marco.silipo on 31.08.2021.
//
#include "vengine.h"
#include "VkBootstrap.h"

#include <GLFW/glfw3.h>

vengine::vengine::vengine() : m_glfw(800, 600, "vengine"), m_initialized(false), m_vulkan_surface(nullptr)
{
    // Create vulkan instance
    auto
            instance_result = vkb::InstanceBuilder { }.set_app_name("vengine")
                                                      .request_validation_layers()
                                                      .use_default_debug_messenger()
                                                      .build();
    if (!instance_result)
    {
        return;
    }
    m_vulkan_instance = instance_result.value();

    // Create vulkan surface
    auto glfw_surface_creation_result = glfwCreateWindowSurface(
            m_vulkan_instance.instance, static_cast<GLFWwindow *>(m_glfw.m_window_handle), nullptr, &m_vulkan_surface);
    if (glfw_surface_creation_result != VK_SUCCESS)
    {
        return;
    }


    // Pick vulkan physical device
    auto
            physical_device_result = vkb::PhysicalDeviceSelector { m_vulkan_instance }.set_surface(m_vulkan_surface)
                                                                                      .set_minimum_version(1, 1)
                                                                                      .require_dedicated_transfer_queue()
                                                                                      .require_present()
                                                                                      .select();
    if (!physical_device_result)
    {
        return;
    }
    m_vulkan_physical_device = physical_device_result.value();


    // Create logical device
    auto device_result = vkb::DeviceBuilder { m_vulkan_physical_device }.build();
    if (!device_result)
    {
        return;
    }
    m_vulkan_device = device_result.value();

    // Create logical device
    auto
            swap_chain_result
            = vkb::SwapchainBuilder { m_vulkan_device }.set_desired_format({ .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                                                       .build();
    if (!swap_chain_result)
    {
        return;
    }
    m_vulkan_swap_chain = swap_chain_result.value();

    // Get Graphics Queue
    auto graphics_queue_result = m_vulkan_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_result)
    {
        return;
    }
    m_vulkan_graphics_queue = graphics_queue_result.value();


    // Set initialized to true
    m_initialized = true;
}

vengine::vengine::~vengine()
{
    if (m_vulkan_swap_chain.swapchain)
    {
        vkb::destroy_swapchain(m_vulkan_swap_chain);
    }
    if (m_vulkan_device.device)
    {
        vkb::destroy_device(m_vulkan_device);
    }
    if (m_vulkan_surface)
    {
        vkb::destroy_surface(m_vulkan_instance, m_vulkan_surface);
    }
    if (m_vulkan_instance.instance)
    {
        vkb::destroy_instance(m_vulkan_instance);
    }
}


#pragma region vengine::vengine::glfw_window
#define glfw_wnd (static_cast<GLFWwindow*>(m_window_handle))

[[maybe_unused]] void vengine::vengine::glfw_window::set_error_callback(void (*error_handle)(int, const char *))
{
    glfwSetErrorCallback(error_handle);
}

vengine::vengine::glfw_window::~glfw_window()
{
    glfw_unset_window_callbacks();
    glfwDestroyWindow(glfw_wnd);
    m_window_handle = nullptr;
    if (m_initialized)
    {
        glfwTerminate();
    }
}

vengine::vengine::glfw_window::glfw_window(int width, int height, const std::string &title)
{
    auto glfw_init_result = glfwInit();
    if (!glfw_init_result)
    {
        m_window_handle = nullptr;
        m_initialized = false;
        return;
    }
    m_initialized = true;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, false);
    m_window_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfw_set_window_callbacks();
}

void vengine::vengine::glfw_window::glfw_unset_window_callbacks()
{
    if (!m_window_handle)
    {
        return;
    }
    glfwSetWindowCloseCallback(glfw_wnd, nullptr);
    glfwSetWindowFocusCallback(glfw_wnd, nullptr);
    glfwSetWindowIconifyCallback(glfw_wnd, nullptr);
    glfwSetWindowMaximizeCallback(glfw_wnd, nullptr);
    glfwSetWindowPosCallback(glfw_wnd, nullptr);
    glfwSetWindowRefreshCallback(glfw_wnd, nullptr);
    glfwSetWindowSizeCallback(glfw_wnd, nullptr);
    glfwSetWindowContentScaleCallback(glfw_wnd, nullptr);
    glfwSetWindowUserPointer(glfw_wnd, nullptr);
}

void vengine::vengine::glfw_window::glfw_set_window_callbacks()
{
    if (!m_window_handle)
    {
        return;
    }
    glfwSetWindowCloseCallback(
            glfw_wnd, [](GLFWwindow *window)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_close.raise(*instance, { });
                }
            });
    glfwSetWindowFocusCallback(
            glfw_wnd, [](GLFWwindow *window, int focused)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_focus.raise(*instance, { static_cast<bool>(focused) });
                }
            });
    glfwSetWindowIconifyCallback(
            glfw_wnd, [](GLFWwindow *window, int iconified)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_iconified.raise(*instance, { static_cast<bool>(iconified) });
                }
            });
    glfwSetWindowMaximizeCallback(
            glfw_wnd, [](GLFWwindow *window, int maximized)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_maximize.raise(*instance, { static_cast<bool>(maximized) });
                }
            });
    glfwSetWindowPosCallback(
            glfw_wnd, [](GLFWwindow *window, int x_pos, int y_pos)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_pos.raise(*instance, { x_pos, y_pos });
                }
            });
    glfwSetWindowRefreshCallback(
            glfw_wnd, [](GLFWwindow *window)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_refresh.raise(*instance, { });
                }
            });
    glfwSetWindowSizeCallback(
            glfw_wnd, [](GLFWwindow *window, int width, int height)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_size.raise(*instance, { width, height });
                }
            });
    glfwSetWindowContentScaleCallback(
            glfw_wnd, [](GLFWwindow *window, float x_scale, float y_scale)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window *>(user_pointer);
                    instance->on_window_content_scale.raise(*instance, { x_scale, y_scale });
                }
            });
    glfwSetWindowUserPointer(glfw_wnd, this);
}

[[maybe_unused]] vengine::vengine::glfw_window::size vengine::vengine::glfw_window::window_size() const
{
    vengine::vengine::glfw_window::size size { 0 };
    glfwGetWindowSize(glfw_wnd, &size.width, &size.height);
    return size;
}

[[maybe_unused]] void vengine::vengine::glfw_window::window_size(const vengine::vengine::glfw_window::size &size)
{
    glfwSetWindowSize(glfw_wnd, size.width, size.height);
}

[[maybe_unused]] void vengine::vengine::glfw_window::window_title(const std::string &title)
{
    glfwSetWindowTitle(glfw_wnd, title.c_str());
}

void vengine::vengine::glfw_window::handle_pending_events()
{
    glfwPollEvents();
}

void vengine::vengine::glfw_window::swap_buffers()
{
    glfwSwapBuffers(glfw_wnd);
}

#pragma endregion