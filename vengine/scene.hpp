//
// Created by marco.silipo on 08.09.2021.
//

#ifndef GAME_PROJ_SCENE_HPP
#define GAME_PROJ_SCENE_HPP

#include "vengine.hpp"
#include <entt/entt.hpp>

namespace vengine
{
    class scene
    {
        vengine &m_engine;
        vengine::on_render_pass_event::event_id m_on_render_pass_event_id;
        entt::registry m_ecs{};
    protected:
        virtual void render_pass(::vengine::vengine::on_render_pass_event_args &args) = 0;

        virtual void load_scene() = 0;

        virtual void unload_scene() = 0;

    public:
        explicit scene(vengine &engine) : m_engine(engine), m_on_render_pass_event_id(vengine::on_render_pass_event::event_id_invalid)
        {

        }

        vengine &engine()
        {
            return m_engine;
        }

        [[nodiscard]] entt::registry& ecs() { return m_ecs; }

        void load()
        {
            m_on_render_pass_event_id = m_engine.on_render_pass.subscribe(
                    [&](auto &sender, auto &args)
                    {
                        render_pass(args);
                    });
            load_scene();
        }

        void unload()
        {
            engine().on_render_pass.unsubscribe(m_on_render_pass_event_id);
            m_on_render_pass_event_id = vengine::on_render_pass_event::event_id_invalid;
            unload_scene();
        }

        class raii_load
        {
            scene &m_scene;
            friend class scene;
        public:
            explicit raii_load(scene &s) : m_scene(s)
            {
                m_scene.load();
            }
            ~raii_load()
            {
                m_scene.unload();
            }
        };
    };
}


#endif //GAME_PROJ_SCENE_HPP
