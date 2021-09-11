//
// Created by marco.silipo on 11.09.2021.
//

#include "test.hpp"
#include "../vengine/vulkan-utils/pipeline_builder.hpp"
#include "../vengine/vulkan-utils/pipeline_layout_builder.hpp"
#include "../vengine/mesh.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void scenes::test::render_pass(vengine::vengine::on_render_pass_event_args &args)
{
    vkCmdBindPipeline(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(args.command_buffer, 0, 1,
            &m_monkey_mesh.vertex_buffer.buffer, &offset);


    // Camera
    // --> Position
    glm::vec3 camPos = { 0.f, 0.f, -2.f };

    glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
    // --> Projection
    glm::mat4 projection = glm::perspective(
            glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;
    // --> Copy to buffer
    vengine::vengine::camera_data{ .view = view, .projection = projection, .view_projection = view * projection }
            .set_gpu_buffer_data(args.current_frame_data.camera_buffer);


    // Model rotation
    glm::mat4 model = glm::rotate(
            glm::mat4{ 1.0f },
            glm::radians((float)engine().frame_count() * 0.01f),
            glm::vec3(0, 1, 0));

    // Calculate final mesh view_matrix
    glm::mat4 mesh_matrix = projection * view * model;

    vengine::mesh::push_constant constants = {};
    constants.matrix = mesh_matrix;

    // Upload the view_matrix to the GPU via push constants
    vkCmdPushConstants(args.command_buffer,
            m_pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(vengine::mesh::push_constant),
            &constants);

    vkCmdDraw(args.command_buffer, (uint32_t)m_monkey_mesh.size(), 1, 0, 0);
}

void scenes::test::load_scene()
{
    m_fragment_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/frag.spv").value()).value();
    m_vertex_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/vert.spv").value()).value();
    m_pipeline_layout = vengine::vulkan_utils::pipeline_layout_builder(engine().vulkan_device())
            .add_push_constant_range(sizeof(vengine::mesh::push_constant), 0, VK_SHADER_STAGE_VERTEX_BIT)
            .build().value();
    m_pipeline = vengine::vulkan_utils::pipeline_builder(
            engine().vulkan_device(),
            engine().vulkan_render_pass(),
            engine().vulkan_default_viewport(),
            engine().vulkan_default_scissors(),
            m_pipeline_layout)
            .add_shader(m_fragment_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_shader(m_vertex_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
            .set_input_assembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .set_rasterization(VkPolygonMode::VK_POLYGON_MODE_FILL)
            .set_multisample()
            .set_pipeline_depths_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
            .add_vertex_input_attribute_descriptions(vengine::vertex::get_vertex_input_description().attribute_descriptions)
            .add_vertex_input_binding_descriptions(vengine::vertex::get_vertex_input_description().binding_descriptions)
            .add_color_blend()
            .build().value();
    m_triangle_mesh = vengine::mesh{
            vengine::vertex { {  1.0f,  1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
            vengine::vertex { { -1.0f,  1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
            vengine::vertex { {  0.0f, -1.0f, 0.0f }, {}, { 0.0f, 1.0f, 0.0f } },
    };
    m_triangle_mesh.upload(engine().allocator());
    m_monkey_mesh = vengine::mesh::from_obj(
            vengine::ram_file::from_disk("assets/monkey_smooth.obj").value(),
            vengine::ram_file::from_disk("assets/monkey_smooth.mtl").value()).value();
    m_monkey_mesh.upload(engine().allocator());
}

void scenes::test::unload_scene()
{
    m_triangle_mesh.destroy();
    m_monkey_mesh.destroy();
    engine().destroy_shader_module(m_fragment_shader);
    engine().destroy_shader_module(m_vertex_shader);
    vkDestroyPipeline(engine().vulkan_device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(engine().vulkan_device(), m_pipeline_layout, nullptr);
    m_triangle_mesh.vertex_buffer.destroy();
    m_monkey_mesh.vertex_buffer.destroy();
}
