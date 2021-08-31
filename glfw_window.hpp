#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace x39
{
    class glfw_window
    {
    private:
        GLFWwindow* m_window_handle;
    protected:

        virtual void on_window_close() { }
        virtual void on_window_focus(bool focused) { }
        virtual void on_window_iconified(bool iconified) { }
        virtual void on_window_maximize(bool maximized) { }
        virtual void on_window_pos(int x_pos, int y_pos) { }
        virtual void on_window_refresh() { }
        virtual void on_window_size(int width, int height) { }
        virtual void on_window_content_scale(float x_scale, float y_scale) { }

        void set_window_callbacks()
        {
            glfwSetWindowCloseCallback(m_window_handle, [](GLFWwindow* window) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_close();
                }
            });
            glfwSetWindowFocusCallback(m_window_handle, [](GLFWwindow* window, int focused) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_focus(focused);
                }
            });
            glfwSetWindowIconifyCallback(m_window_handle, [](GLFWwindow* window, int iconified) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_iconified(iconified);
                }
            });
            glfwSetWindowMaximizeCallback(m_window_handle, [](GLFWwindow* window, int maximized) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_maximize(maximized);
                }
            });
            glfwSetWindowPosCallback(m_window_handle, [](GLFWwindow* window, int x_pos, int y_pos) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_pos(x_pos, y_pos);
                }
            });
            glfwSetWindowRefreshCallback(m_window_handle, [](GLFWwindow* window) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_refresh();
                }
            });
            glfwSetWindowSizeCallback(m_window_handle, [](GLFWwindow* window, int width, int height) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_size(width, height);
                }
            });
            glfwSetWindowContentScaleCallback(m_window_handle, [](GLFWwindow* window, float x_scale, float y_scale) {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<glfw_window*>(user_pointer);
                    instance->on_window_content_scale(x_scale, y_scale);
                }
            });
            glfwSetWindowUserPointer(m_window_handle, this);
        }
        void unset_window_callbacks()
        {
            glfwSetWindowCloseCallback(m_window_handle, nullptr);
            glfwSetWindowFocusCallback(m_window_handle, nullptr);
            glfwSetWindowIconifyCallback(m_window_handle, nullptr);
            glfwSetWindowMaximizeCallback(m_window_handle, nullptr);
            glfwSetWindowPosCallback(m_window_handle,nullptr);
            glfwSetWindowRefreshCallback(m_window_handle, nullptr);
            glfwSetWindowSizeCallback(m_window_handle, nullptr);
            glfwSetWindowContentScaleCallback(m_window_handle, nullptr);
            glfwSetWindowUserPointer(m_window_handle, nullptr);
        }
    public:
        glfw_window(int width, int height, const std::string& title)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, false);
            m_window_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            set_window_callbacks();
        }

        virtual ~glfw_window()
        {
            unset_window_callbacks();
            glfwDestroyWindow(m_window_handle);
        }

        [[nodiscard]] GLFWwindow* window_handle() const { return m_window_handle; }
        [[nodiscard]] bool good() const { return m_window_handle != nullptr; }

        void loop()
        {
            while (!glfwWindowShouldClose(m_window_handle))
            {
                glfwPollEvents();
            }
        }
    };
}