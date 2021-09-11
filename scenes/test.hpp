//
// Created by marco.silipo on 11.09.2021.
//

#ifndef GAME_PROJ_TEST_HPP
#define GAME_PROJ_TEST_HPP

#include "../vengine/scene.hpp"
#include "../vengine/mesh.hpp"

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
    protected:
        void render_pass(::vengine::vengine::on_render_pass_event_args &args) override;
        void load_scene() override;
        void unload_scene() override;
    public:
        explicit test(vengine::vengine& engine) : vengine::scene(engine) {}
    };
}


#endif //GAME_PROJ_TEST_HPP
