#include "vengine/vengine.hpp"
#include "vengine/vulkan-utils/pipeline_builder.hpp"
#include "vengine/vulkan-utils/pipeline_layout_builder.hpp"
#include "vengine/mesh.hpp"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Current Chapter https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Image_views
// Current Chapter https://vkguide.dev/docs/chapter-3/scene_management/


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
    auto pipeline_layout = vengine::vulkan_utils::pipeline_layout_builder(engine.vulkan_device())
            .add_push_constant_range(sizeof(vengine::mesh::push_constant), 0, VK_SHADER_STAGE_VERTEX_BIT)
            .build().value();
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
            .set_pipeline_depths_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
            .add_vertex_input_attribute_descriptions(vengine::vertex::get_vertex_input_description().attribute_descriptions)
            .add_vertex_input_binding_descriptions(vengine::vertex::get_vertex_input_description().binding_descriptions)
            .add_color_blend()
            .build().value();
    vengine::mesh triangle_mesh = {
                    vengine::vertex { {  1.0f,  1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
                    vengine::vertex { { -1.0f,  1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
                    vengine::vertex { {  0.0f, -1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
            };
    triangle_mesh.upload(engine.allocator());
    vengine::mesh monkey_mesh = vengine::mesh::from_obj(
            vengine::ram_file::from_disk("assets/monkey_smooth.obj").value(),
            vengine::ram_file::from_disk("assets/monkey_smooth.mtl").value()).value();
    monkey_mesh.upload(engine.allocator());
    engine.on_render_pass.subscribe([&](auto& source, auto& args) {
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd_buffer, 0, 1,
                &monkey_mesh.vertex_buffer.buffer, &offset);
        // Make a model view matrix for rendering the object
        // Camera position
        glm::vec3 camPos = { 0.f,0.f,-2.f };

        glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        // Camera projection
        glm::mat4 projection = glm::perspective(
                glm::radians(70.f),
                1700.f / 900.f,
                0.1f,
                200.0f);
        projection[1][1] *= -1;
        // Model rotation
        glm::mat4 model = glm::rotate(
                glm::mat4{ 1.0f },
                glm::radians((float)source.frame_count() * 0.01f),
                glm::vec3(0, 1, 0));

        // Calculate final mesh matrix
        glm::mat4 mesh_matrix = projection * view * model;

        vengine::mesh::push_constant constants = {};
        constants.matrix = mesh_matrix;

        //upload the matrix to the GPU via push constants
        vkCmdPushConstants(cmd_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(vengine::mesh::push_constant),
                &constants);

        vkCmdDraw(cmd_buffer, (uint32_t)monkey_mesh.size(), 1, 0, 0);
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
        triangle_mesh.vertex_buffer.destroy();
        monkey_mesh.vertex_buffer.destroy();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}