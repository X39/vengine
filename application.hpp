#pragma once
#include "vulkan.hpp"
#include "glfw_window.hpp"

#include <iostream>

namespace x39
{
    class application : public glfw_window, public vulkan
    {
    protected:

        void on_window_close() override
        {
            vulkan_destroy();
        }

        [[nodiscard]] std::vector<std::string> get_expected_validation_layers() const override { return { "VK_LAYER_KHRONOS_validation" }; }
        void on_debug_message(severity severity, debug_message_type type, debug_callback_data* data) override
        {
            switch (severity)
            {
                case severity::error:
                    std::cerr << "[" << to_string(severity) << "]" << "    " << data->pMessage << std::endl;
                    break;
                default:
                    std::cout << "[" << to_string(severity) << "]" << "    " << data->pMessage << std::endl;
                    break;
            }
        }
    public:
        application() :
                glfw_window(800, 600, "Vulkan"),
                vulkan(glfw_window::window_handle())
        {
            vulkan_initialize("Vulkan", { 0, 0, 0 }, "Unnamed", { 0, 0, 0 });
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
        [[nodiscard]] bool good() const { return glfw_window::good() && vulkan::good(); }
        [[nodiscard]] std::vector<std::string> errors() const
#pragma clang diagnostic pop
        {
            std::vector<std::string> tmp;
            if (!glfw_window::good())
            {
                tmp.emplace_back("Failed to create window using GLFW.");
            }
            for (const auto& err : vulkan::errors())
            {
                tmp.push_back(std::string(to_string(err.result)).append(" ").append(err.info));
            }
            return tmp;
        }
    };
}