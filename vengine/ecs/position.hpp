//
// Created by marco.silipo on 15.09.2021.
//

#ifndef GAME_PROJ_POSITION_HPP
#define GAME_PROJ_POSITION_HPP

namespace vengine::ecs
{
    struct position
    {
        glm::vec3 data{};

        operator glm::vec3() const { return data; } // NOLINT(google-explicit-constructor)
        operator glm::vec3&() { return data; } // NOLINT(google-explicit-constructor)
    };
}

#endif //GAME_PROJ_POSITION_HPP
