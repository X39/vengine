#include "vulkan.hpp"
#include "application.hpp"
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>

// Current Chapter https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Image_views


void glfw_error(int error_code, const char *error_message)
{
    std::cerr << "GLFW ERROR [" << std::hex << error_code << "]: " << error_message << std::endl;
}

int main(int argc, char **argv)
{
    struct RAIIGlfwTerminate
    {
        ~RAIIGlfwTerminate()
        {
            glfwTerminate();
        }
    };
    glfwSetErrorCallback(glfw_error);
    if (glfwInit() == GLFW_FALSE)
    {
        return EXIT_FAILURE;
    }
    RAIIGlfwTerminate _glfw_;


    // Create Application
    x39::application app;

    if (!app.good())
    {
        for (const auto &msg : app.errors())
        {
            std::cerr << msg << std::endl;
        }
    }


    if (!std::filesystem::exists("diag/capabilities.txt"))
    {
        /* Print out GLFW info */
        std::ofstream diagnostics("diag/capabilities.txt");
        if (diagnostics.good())
        {
            diagnostics << "GLFW Version: " << glfwGetVersionString() << std::endl;

            /* Print supported vulkan stuff */
            {
                diagnostics << "Device extensions:" << std::endl;
                size_t index = 0;
                for (auto device : app.get_physical_devices())
                {
                    index++;
                    diagnostics << "    - " << index++ << ": " << std::endl;
                    auto supported_extensions = x39::application::get_device_extension_properties(device);
                    std::sort(
                            supported_extensions.begin(),
                            supported_extensions.end(),
                            [](VkExtensionProperties left, VkExtensionProperties right)
                            {
                                return std::string_view(left.extensionName) < (std::string_view(right.extensionName));
                            });
                    for (auto it : supported_extensions)
                    {
                        diagnostics << "        - " << it.extensionName << std::endl;
                    }
                }
                diagnostics << "Device layers:" << std::endl;
                index = 0;
                for (auto device : app.get_physical_devices())
                {
                    index++;
                    diagnostics << "    - " << index++ << ": " << std::endl;
                    auto supported_extensions = x39::application::get_device_layer_properties(device);
                    std::sort(
                            supported_extensions.begin(),
                            supported_extensions.end(),
                            [](VkLayerProperties left, VkLayerProperties right)
                            {
                                return std::string_view(left.layerName) < (std::string_view(right.layerName));
                            });
                    for (auto it : supported_extensions)
                    {
                        diagnostics << "        - " << it.layerName << ": " << it.description << std::endl;
                    }
                }
                diagnostics << "Instance extensions:" << std::endl;
                for (auto it : x39::application::get_instance_extension_properties())
                {
                    diagnostics << "    - " << it.extensionName << std::endl;
                }
                diagnostics << "Instance layers:" << std::endl;
                for (auto it : x39::application::get_instance_layer_properties())
                {
                    diagnostics << "    - " << it.layerName << ": " << it.description << std::endl;
                }
            }
        }
    }

    if (!app.good())
    {
        return EXIT_FAILURE;
    }

    // Run Application Loop
    try
    {
        app.loop();
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}