//
// Created by marco.silipo on 10.09.2021.
//

#ifndef GAME_PROJ_ROTATION_HPP
#define GAME_PROJ_ROTATION_HPP

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <cmath>

namespace vengine::ecs
{
    struct rotation
    {
        glm::quat data{};
        operator glm::quat() const { return data; } // NOLINT(google-explicit-constructor)
        operator glm::quat&() { return data; } // NOLINT(google-explicit-constructor)


        [[maybe_unused]] [[nodiscard]] glm::vec3 euler_angles() const
        {
            return glm::eulerAngles(data);
        }

        [[maybe_unused]] [[nodiscard]] float euler_pitch() const
        {
            return euler_angles().x;
        }

        [[maybe_unused]] [[nodiscard]] float euler_yaw() const
        {
            return euler_angles().y;
        }

        [[maybe_unused]] [[nodiscard]] float euler_roll() const
        {
            return euler_angles().z;
        }

        void euler_angles(glm::vec3 euler)
        {
            data = glm::quat { euler };
        }
        void euler_angles(float pitch, float yaw, float roll)
        {
            euler_angles({pitch, yaw, roll});
        }
    };
}

#endif //GAME_PROJ_ROTATION_HPP
