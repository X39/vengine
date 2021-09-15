//
// Created by marco.silipo on 11.09.2021.
//

#include "test.hpp"
#include "../vengine/vulkan-utils/pipeline_builder.hpp"
#include "../vengine/vulkan-utils/pipeline_layout_builder.hpp"
#include "../vengine/mesh.hpp"
#include "../vengine/ecs/position.hpp"
#include "../vengine/ecs/rotation.hpp"
#include "../vengine/ecs/renderable.hpp"
#include "../vengine/ecs/velocity.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

void scenes::test::render_pass(vengine::vengine::on_render_pass_event_args &args)
{
    vkCmdBindPipeline(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(args.command_buffer, 0, 1, &m_monkey_mesh.vertex_buffer.buffer, &offset);


    {
        // Camera
        // --> Position
        glm::vec3 camPos = { 0.f, 0.f, -2.f };
        glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        // --> Projection
        glm::mat4 projection = glm::perspective(
                glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
        projection[1][1] *= -1;
        // --> Copy to buffer
        vengine::vengine::gpu_camera_data{ .view = view, .projection = projection, .view_projection = view * projection }
                .set_gpu_buffer_data(args.current_frame_data.camera_buffer);
    }
    // auto camera_position = ecs().get<vengine::ecs::position>(m_camera);
    // auto camera_velocity = ecs().get<vengine::ecs::velocity>(m_camera);
    // auto camera_rotation = ecs().get<vengine::ecs::rotation>(m_camera);
    // engine().current_frame_data().camera_buffer.with_mapped(
    //         [&](auto &span)
    //         {
    //             auto camera_data = reinterpret_cast<vengine::vengine::gpu_camera_data *>(span.data());
    //             camera_data->view = glm::mat4_cast(camera_rotation.data)
    //                                 * glm::translate(glm::mat4 { 1.0f }, -camera_position.data);
    //             glm::mat4 projection = glm::perspective(
    //                     glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    //             projection[1][1] *= -1;
    //             camera_data->projection = projection;
    //             camera_data->view_projection = camera_data->view * projection;
    //         });

    ecs().view<vengine::ecs::position, vengine::ecs::velocity>().each(
                 [&](auto &pos, auto &vel)
                 {
                     pos.data += vel.data;
                     vel.data *= 0.25;
                 });


    size_t meshes = 0;
    engine().current_frame_data().mesh_buffer.with_mapped(
            [&](auto &span)
            {
                auto mesh_data = reinterpret_cast<vengine::vengine::gpu_mesh_data **>(span.data());
                size_t index = 0;
                ecs().view<const vengine::ecs::position, const vengine::ecs::rotation, const vengine::ecs::renderable>()
                     .each(
                             [&](const auto &pos, const auto &rot, const auto &renderable)
                             {
                                 mesh_data[index++]->matrix = glm::mat4_cast(rot.data)
                                                              * glm::translate(glm::mat4 { 1.0f }, pos.data)
                                                              * glm::scale(glm::mat4 { 1.0f }, renderable.scale);
                                 meshes++;
                             });
            });


    uint32_t render_index = 0;
    vengine::mesh *current_mesh { };
    ecs().view<const vengine::ecs::position, const vengine::ecs::rotation, const vengine::ecs::renderable>().each(
                 [&](const auto &pos, const auto &rot, const auto &renderable)
                 {
                     vengine::mesh::push_constant constants = { };
                     constants.matrix = glm::mat4_cast(rot.data)
                                        * glm::translate(glm::mat4 { 1.0f }, pos.data)
                                        * glm::scale(glm::mat4 { 1.0f }, renderable.scale);

                     // Upload the view_matrix to the GPU via push constants
                     vkCmdPushConstants(
                             args.command_buffer,
                             m_pipeline_layout,
                             VK_SHADER_STAGE_VERTEX_BIT,
                             0,
                             sizeof(vengine::mesh::push_constant),
                             &constants);

                     if (renderable.mesh != current_mesh)
                     {
                         VkDeviceSize offset = 0;
                         vkCmdBindVertexBuffers(
                                 args.command_buffer,
                                 0,
                                 1,
                                 &renderable.mesh->vertex_buffer.buffer,
                                 &offset);
                         current_mesh = renderable.mesh;
                     }
                     vkCmdDraw(args.command_buffer, (uint32_t) current_mesh->vertices.size(), 1, 0, render_index++);
                 });
}

void scenes::test::load_scene()
{
    m_fragment_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/frag.spv").value()).value();
    m_vertex_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/vert.spv").value()).value();
    m_pipeline_layout
            = vengine::vulkan_utils::pipeline_layout_builder(engine().vulkan_device()).add_push_constant_range(
                                                                                              sizeof(vengine::mesh::push_constant),
                                                                                              0,
                                                                                              VK_SHADER_STAGE_VERTEX_BIT)
                                                                                      .add_descriptor_set_layout(
                                                                                              engine().vulkan_descriptor_set_layout())
                                                                                      .build()
                                                                                      .value();
    m_pipeline = vengine::vulkan_utils::pipeline_builder(
            engine().vulkan_device(),
            engine().vulkan_render_pass(),
            engine().vulkan_default_viewport(),
            engine().vulkan_default_scissors(),
            m_pipeline_layout).add_shader(m_fragment_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
                              .add_shader(m_vertex_shader, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
                              .set_input_assembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                              .set_rasterization(VkPolygonMode::VK_POLYGON_MODE_FILL)
                              .set_multisample()
                              .set_pipeline_depths_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
                              .add_vertex_input_attribute_descriptions(vengine::vertex::get_vertex_input_description().attribute_descriptions)
                              .add_vertex_input_binding_descriptions(vengine::vertex::get_vertex_input_description().binding_descriptions)
                              .add_color_blend()
                              .build()
                              .value();
    m_triangle_mesh = vengine::mesh {
            vengine::vertex {
                    { 1.0f, 1.0f, 0.0f },
                    { },
                    { 0.0f, 1.0f, 0.0f } },
            vengine::vertex {
                    { -1.0f, 1.0f, 0.0f },
                    { },
                    { 0.0f,  1.0f, 0.0f } },
            vengine::vertex {
                    { 0.0f, -1.0f, 0.0f },
                    { },
                    { 0.0f, 1.0f,  0.0f } }, };
    m_triangle_mesh.upload(engine().allocator());
    m_monkey_mesh = vengine::mesh::from_obj(
            vengine::ram_file::from_disk("assets/monkey_smooth.obj").value(),
            vengine::ram_file::from_disk("assets/monkey_smooth.mtl").value()).value();
    m_monkey_mesh.upload(engine().allocator());

    {
        vengine::ecs::position pos { };
        pos.data = { 0, 0, 0 };
        vengine::ecs::rotation rot { };
        rot.euler_angles(0, 0, 0);
        vengine::ecs::velocity vel { };
        vel.data = { 0, 0, 0 };
        m_camera = ecs().create();
        ecs().emplace<vengine::ecs::position>(m_camera, pos);
        ecs().emplace<vengine::ecs::rotation>(m_camera, rot);
        ecs().emplace<vengine::ecs::velocity>(m_camera, vel);
    }

    for (size_t x = -1000; x <= 1000; x++)
    {
        for (size_t y = -1000; y <= 1000; y++)
        {
            vengine::ecs::position pos { };
            pos.data = { x, y, 0 };
            vengine::ecs::rotation rot { };
            rot.euler_angles(0, 0, 0);
            vengine::ecs::velocity vel { };
            vel.data = { x, y, 0 };
            vengine::ecs::renderable renderable { };
            renderable.mesh = &m_monkey_mesh;

            auto entity = ecs().create();
            ecs().emplace<vengine::ecs::position>(entity, pos);
            ecs().emplace<vengine::ecs::rotation>(m_camera, rot);
            ecs().emplace<vengine::ecs::velocity>(entity, vel);
            ecs().emplace<vengine::ecs::renderable>(entity, renderable);
        }
    }

    engine().on_key.subscribe(
            [&](auto &sender, auto &args)
            {
                auto& camera_velocity = ecs().get<vengine::ecs::velocity>(m_camera);
                auto& camera_position = ecs().get<vengine::ecs::position>(m_camera);
                auto& camera_rotation = ecs().get<vengine::ecs::rotation>(m_camera);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
                switch (args.key)
#pragma clang diagnostic pop
                {
                    case vengine::vengine::keys::KEY_W:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(1, 0, 0);
                        break;
                    case vengine::vengine::keys::KEY_A:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(0, 1, 0);
                        break;
                    case vengine::vengine::keys::KEY_S:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(-1, 0, 0);
                        break;
                    case vengine::vengine::keys::KEY_D:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(0, -1, 0);
                        break;
                    case vengine::vengine::keys::KEY_SPACE:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(0, 0, 1);
                        break;
                    case vengine::vengine::keys::KEY_LEFT_CONTROL:
                        camera_velocity.data += camera_rotation.data
                                                * glm::vec3(0, 0, -1);
                        break;
                }
            });
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
