//
// Created by marco.silipo on 11.09.2021.
//

#ifndef GAME_PROJ_TEST_HPP
#define GAME_PROJ_TEST_HPP

#include "../vengine/scene.hpp"
#include "../vengine/mesh.hpp"
#include "../vengine/vengine.hpp"

namespace scenes
{
    class test : public vengine::scene
    {
        VkShaderModule m_fragment_shader{};
        VkShaderModule m_vertex_shader{};
        VkPipelineLayout m_pipeline_layout{};
        VkPipeline m_pipeline{};
        vengine::mesh m_triangle_mesh;
        vengine::mesh m_monkey_mesh;
        bool m_can_rotate;
        entt::entity m_camera;

        void callback_mouse_button(vengine::vengine& engine, vengine::vengine::on_mouse_button_event_args& args);
        void callback_mouse_move(vengine::vengine& engine, vengine::vengine::on_mouse_move_event_args& args);
    protected:
        void render_pass(::vengine::vengine::on_render_pass_event_args &args) override;
        void load_scene() override;
        void unload_scene() override;

        void handle_player_input();
        glm::mat4 set_camera();
    public:
        explicit test(vengine::vengine& engine) : vengine::scene(engine), m_can_rotate(false) {}

    };
}


#endif //GAME_PROJ_TEST_HPP
