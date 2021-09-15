#include "vengine/log.hpp"
#include "vengine/vengine.hpp"
#include "scenes/test.hpp"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>

// Current Chapter https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Image_views
// Current Chapter https://vkguide.dev/docs/chapter-4/storage_buffers/


void glfw_error(int error_code, const char *error_message)
{
    std::cerr << "GLFW ERROR [" << std::hex << error_code << "]: " << error_message << std::endl;
}

int main(int argc, char **argv)
{
    vengine::log::info("main(int, char**)", "Starting...");
    vengine::vengine engine;
    if (!engine.good())
    {
        return EXIT_FAILURE;
    }


    bool alive = true;
    engine.on_window_close.subscribe([&](auto& source, auto& args) { alive = false; });
    // Run Application Loop
    try
    {
        scenes::test t(engine);
        scenes::test::raii_load t_raii(t);
        size_t old_fps_count = 0;
        auto old_ts = std::chrono::system_clock::now();
        while (alive)
        {
            vengine::vengine::handle_pending_events();
            auto render_result = engine.render();
            if (!render_result) { break; }
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