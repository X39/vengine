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

#include <iomanip>
#include <iostream>

glm::mat4 scenes::test::set_camera()
{
    auto camera_position = ecs().get<vengine::ecs::position>(m_camera);
    auto camera_velocity = ecs().get<vengine::ecs::velocity>(m_camera);
    auto camera_rotation = ecs().get<vengine::ecs::rotation>(m_camera);



    glm::mat4 cam_translate = glm::translate(glm::mat4{1.0}, camera_position.data);
    glm::mat4 cam_rotate = mat4_cast(camera_rotation.data);
    glm::mat4 cam_scale = glm::scale(glm::mat4{1.0f}, glm::vec3{1, 1, 1});

    glm::mat4 view = cam_scale * cam_rotate * cam_translate;
    //std::cout << "{ "
    //          << "p-r: " << std::setw(8)<< std::setfill(' ') << camera_rotation.euler_pitch() << ", "
    //          << "y-r: " << std::setw(8)<< std::setfill(' ') << camera_rotation.euler_yaw()   << ", "
    //          << "r-r: " << std::setw(8)<< std::setfill(' ') << camera_rotation.euler_roll()  << " } - "
    //          << "{ "
    //          << "p-d: " << std::setw(8)<< std::setfill(' ') << glm::degrees(camera_rotation.euler_pitch()) << ", "
    //          << "y-d: " << std::setw(8)<< std::setfill(' ') << glm::degrees(camera_rotation.euler_yaw())   << ", "
    //          << "r-d: " << std::setw(8)<< std::setfill(' ') << glm::degrees(camera_rotation.euler_roll())  << " } - "
    //          << "{ "
    //          << "x: " << std::setw(8)<< std::setfill(' ') << camera_position.data.x << ", "
    //          << "y: " << std::setw(8)<< std::setfill(' ') << camera_position.data.y << ", "
    //          << "z: " << std::setw(8)<< std::setfill(' ') << camera_position.data.z << " }" << std::endl;
    // --> Projection
    glm::mat4 projection = glm::perspective(
            glm::radians(40.f),
            (float)engine().vulkan_default_scissors().extent.width / (float)engine().vulkan_default_scissors().extent.height,
            0.1f, 5000.0f);
    projection[1][1] *= -1;

    auto projection_view = projection * view;
    engine().current_frame_data().camera_buffer.with_mapped(
            [&](auto &span)
            {
                auto camera_data = reinterpret_cast<vengine::vengine::gpu_camera_data *>(span.data());
                camera_data->view = view;
                camera_data->projection = projection;
                camera_data->view_projection = projection * view;
            });
    return projection_view;
}

void scenes::test::render_pass(vengine::vengine::on_render_pass_event_args &args)
{
    handle_player_input();
    vkCmdBindPipeline(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &args.current_frame_data.descriptor_set, 0, NULL);

    auto projection_view = set_camera();

    ecs().view<vengine::ecs::position, vengine::ecs::velocity>().each(
                 [&](auto &pos, auto &vel)
                 {
                     pos.data += vel.data;
                     vel.data *= 0.25;
                 });


    engine().current_frame_data().mesh_buffer.with_mapped(
            [&](std::span<uint8_t>& span)
            {
                auto mesh_data = reinterpret_cast<vengine::vengine::gpu_mesh_data*>(span.data());
                uint32_t render_index = 0;
                vengine::mesh *current_mesh { };
                auto view = ecs().view<const vengine::ecs::position, const vengine::ecs::rotation, const vengine::ecs::renderable>();
                for (auto [entity, pos, rot, renderable] : view.each())
                {
                    auto scale = glm::scale(glm::mat4 { 1.0f }, renderable.scale);
                    auto rotate = glm::mat4_cast(rot.data);
                    auto translate = glm::translate(glm::mat4 { 1.0f }, pos.data);
                    auto model = translate * rotate * scale;
                    mesh_data[render_index].matrix = model;
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
                }
            });
}

void scenes::test::load_scene()
{
    m_fragment_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/frag.spv").value()).value();
    m_vertex_shader = engine().create_shader_module(vengine::ram_file::from_disk("shaders/vert.spv").value()).value();
    m_pipeline_layout = vengine::vulkan_utils::pipeline_layout_builder(engine().vulkan_device())
            // .add_push_constant_range(sizeof(vengine::mesh::push_constant),0,VK_SHADER_STAGE_VERTEX_BIT)
            .add_descriptor_set_layout(engine().vulkan_descriptor_set_layout())
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
    m_triangle_mesh.upload_to_gpu_memory(engine(), engine().allocator());
    m_monkey_mesh = vengine::mesh::from_obj(
            vengine::ram_file::from_disk("assets/monkey_smooth.obj").value(),
            vengine::ram_file::from_disk("assets/monkey_smooth.mtl").value()).value();
    m_monkey_mesh.upload_to_gpu_memory(engine(), engine().allocator());

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

    const int max = 22;
    const int mul = 5;
    for (int x = -max; x <= max; x++)
    {
        for (int y = -max; y <= max; y++)
        {
            for (int z = -max; z <= max; z++)
            {
                vengine::ecs::position pos { };
                pos.data = { x*mul, y*mul, z*mul };
                vengine::ecs::rotation rot { };
                rot.euler_angles(0, 0, 0);
                vengine::ecs::velocity vel { };
                vel.data = { 0, 0, 0 };
                vengine::ecs::renderable renderable { };
                renderable.mesh = &m_monkey_mesh;
                renderable.scale = glm::vec3{1, 1, 1};

                auto entity = ecs().create();
                ecs().emplace<vengine::ecs::position>(entity, pos);
                ecs().emplace<vengine::ecs::rotation>(entity, rot);
                ecs().emplace<vengine::ecs::velocity>(entity, vel);
                ecs().emplace<vengine::ecs::renderable>(entity, renderable);
            }
        }
    }

    engine().on_mouse_button.subscribe([&](auto& sender, auto& args) { callback_mouse_button(sender, args); });
    engine().on_mouse_move.subscribe([&](auto& sender, auto& args) { callback_mouse_move(sender, args); });
}

void scenes::test::callback_mouse_button(vengine::vengine& engine, vengine::vengine::on_mouse_button_event_args& args)
{
    m_can_rotate = args.action == vengine::vengine::key_actions::PRESS;
}
void scenes::test::callback_mouse_move(vengine::vengine& engine, vengine::vengine::on_mouse_move_event_args& args)
{
    if (!m_can_rotate) { return; }
    auto &camera_rotation = ecs().get<vengine::ecs::rotation>(m_camera);
    const bool FPS_CAM = false;
    if (FPS_CAM)
    {
    }
    else
    {
        auto pitch = glm::vec3(glm::radians((float) (args.delta_y)), 0.0f, 0.0f);
        auto yaw = glm::vec3(0.0f, glm::radians((float) (args.delta_x)), 0.0f);
        auto pitch_adjusted = pitch * camera_rotation.data;
        auto yaw_adjusted = yaw * camera_rotation.data;
        camera_rotation.data = camera_rotation.data * glm::quat(pitch_adjusted + yaw_adjusted);
    }
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

void scenes::test::handle_player_input()
{
    auto &camera_velocity = this->ecs().get<vengine::ecs::velocity>(this->m_camera);
    auto &camera_position = this->ecs().get<vengine::ecs::position>(this->m_camera);
    auto &camera_rotation = this->ecs().get<vengine::ecs::rotation>(this->m_camera);
    glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
    float mod = 1.0f;
    if (engine().get_key(vengine::vengine::keys::KEY_Q) == vengine::vengine::key_actions::PRESS)
    {
        auto roll = glm::quat(
                glm::vec3(0.0f, 0.0f, glm::radians(-2.0f))
                * camera_rotation.data);
        camera_rotation.data = camera_rotation.data * roll;
    }
    if (engine().get_key(vengine::vengine::keys::KEY_E) == vengine::vengine::key_actions::PRESS)
    {
        auto roll = glm::quat(
                glm::vec3(0.0f, 0.0f, glm::radians(2.0f))
                * camera_rotation.data);
        camera_rotation.data = camera_rotation.data * roll;
    }
    if (engine().get_key(vengine::vengine::keys::KEY_W) == vengine::vengine::key_actions::PRESS)            { direction += glm::vec3(0, 0, 1); }
    if (engine().get_key(vengine::vengine::keys::KEY_A) == vengine::vengine::key_actions::PRESS)            { direction += glm::vec3(1, 0, 0); }
    if (engine().get_key(vengine::vengine::keys::KEY_S) == vengine::vengine::key_actions::PRESS)            { direction += glm::vec3(0, 0, -1); }
    if (engine().get_key(vengine::vengine::keys::KEY_D) == vengine::vengine::key_actions::PRESS)            { direction += glm::vec3(-1, 0, 0); }
    if (engine().get_key(vengine::vengine::keys::KEY_SPACE) == vengine::vengine::key_actions::PRESS)        { direction += glm::vec3(0, -1, 0); }
    if (engine().get_key(vengine::vengine::keys::KEY_LEFT_CONTROL) == vengine::vengine::key_actions::PRESS) { direction += glm::vec3(0, 1, 0); }
    if (engine().get_key(vengine::vengine::keys::KEY_LEFT_SHIFT) == vengine::vengine::key_actions::PRESS) { mod += 2.0f; }

    camera_velocity.data += direction * mod * camera_rotation.data;
}
