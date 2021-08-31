#include "vengine/vengine.h"

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
    vengine::vengine engine;
    if (!engine.good())
    {
        return EXIT_FAILURE;
    }

    bool alive = true;
    engine.window().on_window_close.subscribe([&](auto& source, auto& args) { alive = false; });
    // Run Application Loop
    try
    {
        while (alive)
        {
            engine.window().handle_pending_events();

            engine.window().swap_buffers();
        }
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}