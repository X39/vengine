#include "vengine/vengine.h"
#include "vengine/vulkan-utils/pipeline_builder.hpp"
#include "vengine/vulkan-utils/pipeline_layout_builder.hpp"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>

// Current Chapter https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Image_views
// Current Chapter https://vkguide.dev/docs/chapter-3/triangle_mesh/


void glfw_error(int error_code, const char *error_message)
{
    std::cerr << "GLFW ERROR [" << std::hex << error_code << "]: " << error_message << std::endl;
}

int main(int argc, char **argv)
{
    vengine::vengine engine;
    if (!engine.good())
    {
        for (auto& err : engine.get_errors())
        {
            std::cerr << err << std::endl;
        }
        return EXIT_FAILURE;
    }

    auto cmd_buffer = engine.create_command_buffer().value();
    auto fragment_shader = engine.create_shader_module(vengine::ram_file::from_disk("shaders/frag.spv").value()).value();
    auto vertex_shader = engine.create_shader_module(vengine::ram_file::from_disk("shaders/vert.spv").value()).value();
    auto pipeline_layout = vengine::vulkan_utils::pipeline_layout_builder(engine.vulkan_device()).build().value();
    auto pipeline = vengine::vulkan_utils::pipeline_builder(
            engine.vulkan_device(),
            engine.vulkan_render_pass(),
            engine.vulkan_default_viewport(),
            engine.vulkan_default_scissors(),
            pipeline_layout)
            .add_shader(fragment_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_shader(vertex_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
            .set_input_assembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .set_rasterization(VkPolygonMode::VK_POLYGON_MODE_FILL)
            .set_multisample()
            .set_vertex_input()
            .add_color_blend()
            .build().value();
    engine.on_render_pass.subscribe([&](auto& source, auto& args) {
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
    });

    bool alive = true;
    engine.on_window_close.subscribe([&](auto& source, auto& args) { alive = false; });
    // Run Application Loop
    try
    {
        size_t old_fps_count = 0;
        auto old_ts = std::chrono::system_clock::now();
        while (alive)
        {
            vengine::vengine::handle_pending_events();
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