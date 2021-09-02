#include "vengine/vengine.h"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>

// Current Chapter https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Image_views
// Current Chapter https://vkguide.dev/docs/chapter-2/triangle_walkthrough/


void glfw_error(int error_code, const char *error_message)
{
    std::cerr << "GLFW ERROR [" << std::hex << error_code << "]: " << error_message << std::endl;
}

int main(int argc, char **argv)
{
    vengine::vengine engine;
    if (!engine.good())
    {
        for (auto err : engine.get_errors())
        {
            std::cerr << err << std::endl;
        }
        return EXIT_FAILURE;
    }

    auto cmd_buffer = engine.create_command_buffer();
    auto fragment_shader = engine.create_shader_module(vengine::ram_file::from_disk("shaders/frag.spv").value());
    auto vertex_shader = engine.create_shader_module(vengine::ram_file::from_disk("shaders/vert.spv").value());

    bool alive = true;
    engine.on_window_close.subscribe([&](auto& source, auto& args) { alive = false; });
    // Run Application Loop
    try
    {
        size_t old_fps_count = 0;
        auto old_ts = std::chrono::system_clock::now();
        while (alive)
        {
            engine.handle_pending_events();
            engine.render();
            engine.swap_buffers();
            auto new_ts = std::chrono::system_clock::now();
            if (new_ts - old_ts > std::chrono::seconds(1))
            {
                old_ts = new_ts;
                auto frame_count = engine.frame_count();
                std::cout << "FPS: " << frame_count - old_fps_count << std::endl;
                old_fps_count = frame_count;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}