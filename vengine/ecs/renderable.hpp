//
// Created by marco.silipo on 10.09.2021.
//

#ifndef GAME_PROJ_RENDERABLE_HPP
#define GAME_PROJ_RENDERABLE_HPP
#include "../mesh.hpp"

namespace vengine::ecs
{
    struct renderable
    {
        ::vengine::mesh* mesh{};
        glm::vec3 scale { 1.0f, 1.0f, 1.0f };
    };
}

#endif //GAME_PROJ_RENDERABLE_HPP
