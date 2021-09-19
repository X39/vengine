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

struct camera_helper
{
    glm::mat4 projection;
    glm::mat4 view;

    void set_orthographic_projection(float left, float top, float right, float bottom, float near, float far)
    {
        projection = glm::mat4{1.0f};
        projection[0][0] = 2.0f / (right - left);
        projection[1][1] = 2.0f / (bottom - top);
        projection[2][2] = 2.0f / (far - near);
        projection[3][0] = -(right - left) / (right - left);
        projection[3][1] = -(bottom + top) / (bottom - top);
        projection[3][2] = -near / (far - near);
    };
    void set_perspective_projection(float fov_y, float aspect, float near, float far)
    {
        auto tan_fov_y_half = tan(fov_y / 2.0f);
        projection = glm::mat4{1.0f};
        projection[0][0] = 1.0f / (aspect * tan_fov_y_half);
        projection[1][1] = 1.0f / tan_fov_y_half;
        projection[2][2] = far / (far - near);
        projection[2][3] = 1.0f;
        projection[3][2] = -(far * near) / (far - near);
    };
    // https://youtu.be/rvJHkYnAR3w?t=331

    void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
    {
        // Set Orthonormal Basis
        auto w = glm::normalize(direction);
        auto u = glm::normalize(glm::cross(w, up));
        auto v = glm::cross(w, u);
        set_view(position, u, v, w);

    }
    void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    {
        set_view_direction(position, target - position, up);
    }
    void set_view_euler(glm::vec3 position, glm::vec3 euler)
    {
        auto cos_z = glm::cos(euler.z);
        auto sin_z = glm::sin(euler.z);
        auto cos_y = glm::cos(euler.x);
        auto sin_y = glm::sin(euler.x);
        auto cos_x = glm::cos(euler.y);
        auto sin_x = glm::sin(euler.y);
        glm::vec3 u{ cos_x * cos_z + sin_x * sin_y * sin_z, cos_y * sin_z, cos_x * sin_y * sin_z - cos_z * sin_x };
        glm::vec3 v{ cos_z * sin_x * sin_y - cos_x * sin_z, cos_y * cos_z, cos_x * cos_z * sin_y + sin_x * sin_z };
        glm::vec3 w{ cos_y * sin_x, -sin_y, cos_x * cos_y };
        set_view(position, u, v, w);
    }
    void set_view_euler(glm::vec3 position, float pitch, float yaw, float roll)
    {
        set_view_euler(position, { pitch, yaw, roll });
    }
private:
    void set_view(glm::vec3 position, glm::vec3 u, glm::vec3 v, glm::vec3 w)
    {
        // Rotation * Translation matrix


        view = glm::mat4{1.0f};

        // <-- Rotation
        view[0][0] = u.x;
        view[0][1] = u.x;
        view[0][2] = u.x;
        view[1][0] = v.x;
        view[1][1] = v.x;
        view[1][2] = v.x;
        view[2][0] = w.x;
        view[2][1] = w.x;
        view[2][2] = w.x;

        // <-- Translation
        view[3][0] = -glm::dot(u, position);
        view[3][1] = -glm::dot(u, position);
        view[3][2] = -glm::dot(u, position);
    }
};

/* WORKING STATE
void scenes::test::render_pass(vengine::vengine::on_render_pass_event_args &args)
{
    vkCmdBindPipeline(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(args.command_buffer, 0, 1,
            &m_monkey_mesh.vertex_buffer.buffer, &offset);


    // Camera
    // --> Position
    // Model rotation
    glm::mat4 cam_rotate = glm::rotate(
            glm::mat4{ 1.0f },
            glm::radians((float)engine().frame_count() * 0.01f),
            glm::vec3(0, 1, 0));
    glm::mat4 cam_translate = glm::translate(glm::mat4{1.0}, glm::vec3{0.0f, 0.0, -2.0});
    glm::mat4 cam_scale = glm::scale(glm::mat4{1.0f}, glm::vec3{1, 0.5, 0.5});

    glm::mat4 view = cam_rotate * cam_translate * cam_scale;
    // --> Projection
    glm::mat4 projection = glm::perspective(
            glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;
    // --> Copy to buffer
    vengine::vengine::camera_data{ .view = view, .projection = projection, .view_projection = view * projection }
            .set_gpu_buffer_data(args.current_frame_data.camera_buffer);


    // Model rotation
    glm::mat4 rotate = glm::rotate(
            glm::mat4{ 1.0f },
            glm::radians((float)engine().frame_count() * 0.005f),
            glm::vec3(0, 1, 0));
    glm::mat4 translate = glm::translate(glm::mat4{1.0}, glm::vec3{1.0f, 0.0, 0.0});
    glm::mat4 scale = glm::scale(glm::mat4{1.0f}, glm::vec3{0.5, 0.5, 0.5});

    // Calculate final mesh view_matrix
    glm::mat4 mesh_matrix = projection * view * (translate * rotate * scale);

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
 */


glm::mat4 scenes::test::set_camera()
{
    // camera_helper cam_helper{};
    // cam_helper.set_perspective_projection(glm::radians(50.0f), 1700.f / 900.f, 0.1f, 200.0f);
    // // cam_helper.set_view_euler(camera_position, camera_rotation.euler_angles());
    // cam_helper.set_view_target(camera_position, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f});

    auto camera_position = ecs().get<vengine::ecs::position>(m_camera);
    auto camera_velocity = ecs().get<vengine::ecs::velocity>(m_camera);
    auto camera_rotation = ecs().get<vengine::ecs::rotation>(m_camera);

    glm::mat4 cam_rotate = glm::rotate(
            glm::mat4{ 1.0f },
            glm::radians((float)engine().frame_count() * 0.01f),
            glm::vec3(0, 1, 0));
    glm::mat4 cam_translate = glm::translate(glm::mat4{1.0}, camera_position.data);

    glm::mat4 view = cam_rotate * cam_translate;
    // --> Projection
    glm::mat4 projection = glm::perspective(
            glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;

    auto projection_view = view * projection;
    engine().current_frame_data().camera_buffer.with_mapped(
            [&](auto &span)
            {
                auto camera_data = reinterpret_cast<vengine::vengine::gpu_camera_data *>(span.data());
                camera_data->view = view;
                camera_data->projection = projection;
                camera_data->view_projection = view * projection;
            });
    return projection_view;
}

void scenes::test::render_pass(vengine::vengine::on_render_pass_event_args &args)
{
    vkCmdBindPipeline(args.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    auto projection_view = set_camera();

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
                auto mesh_data = reinterpret_cast<vengine::vengine::gpu_mesh_data*>(span.data());
                size_t index = 0;
                ecs().view<const vengine::ecs::position, const vengine::ecs::rotation, const vengine::ecs::renderable>()
                     .each(
                             [&](const auto &pos, const auto &rot, const auto &renderable)
                             {
                                 glm::mat4 rotate = glm::rotate(
                                         glm::mat4{ 1.0f },
                                         glm::radians((float)engine().frame_count() * 0.005f),
                                         glm::vec3(0, 1, 0));
                                 auto scale = glm::scale(glm::mat4 { 1.0f }, renderable.scale);
                                 // auto rotate = glm::mat4_cast(rot.data);
                                 auto translate = glm::translate(glm::mat4 { 1.0f }, pos.data);
                                 auto model = translate * rotate * scale;
                                 mesh_data[index++].matrix = model;
                                 meshes++;
                             });
            });


    uint32_t render_index = 0;
    vengine::mesh *current_mesh { };
    ecs().view<const vengine::ecs::position, const vengine::ecs::rotation, const vengine::ecs::renderable>().each(
                 [&](const auto &pos, const auto &rot, const auto &renderable)
                 {
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

    for (int x = -10; x <= 10; x++)
    {
        for (int y = -10; y <= 10; y++)
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
            ecs().emplace<vengine::ecs::rotation>(entity, rot);
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
