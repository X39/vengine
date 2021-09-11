//
// Created by marco.silipo on 10.09.2021.
//

#ifndef GAME_PROJ_POSITION_HPP
#define GAME_PROJ_POSITION_HPP

#include <glm/glm.hpp>
#include <cmath>

namespace vengine::ecs
{
    struct position
    {
        glm::mat4 translation_matrix;
        glm::mat4 rotation_matrix;


        [[maybe_unused]] void look_at(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
        {
            auto z = glm::normalize(position, forward);
            auto x = glm::normalize(glm::cross(up, z));
            auto y = glm::normalize(glm::cross(z, x));
            rotation_matrix = glm::mat4( // Maybe needs columns and rows swapped
                    glm::vec4(x.x, y.y, z.z, 0.0f),
                    glm::vec4(x.x, y.y, z.z, 0.0f),
                    glm::vec4(x.x, y.y, z.z, 0.0f),
                    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            translation_matrix = glm::mat4(
                    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),);
            translation_matrix[3][0] = -position.x;
            translation_matrix[3][1] = -position.y;
            translation_matrix[3][2] = -position.z;
            translation_matrix[3][3] = 1.0f;
        }

        [[maybe_unused]] [[nodiscard]] glm::mat4 view_matrix() const { return translation_matrix * rotation_matrix; }

        void euler_angles(double &pitch, double &yaw, double &roll) const
        {
            pitch = atan2(-rotation_matrix[1][2], rotation_matrix[2][2]);
            auto cos_y_angle = std::sqrt(pow(rotation_matrix[0][0], 2) + pow(rotation_matrix[0][1], 2));
            yaw = atan2(rotation_matrix[0][2], cos_y_angle);
            auto sin_x_angle = sin(pitch);
            auto cos_x_angle = cos(pitch);
            roll = atan2(
                    cos_x_angle * rotation_matrix[1][0] + sin_x_angle * rotation_matrix[2][0],
                    cos_x_angle * rotation_matrix[1][1] + sin_x_angle * rotation_matrix[2][1]);
        }

        [[maybe_unused]] [[nodiscard]] double euler_pitch() const
        {
            double p, y, r;
            euler_angles(p, y, r);
            return p;
        }

        [[maybe_unused]] [[nodiscard]] double euler_yaw() const
        {
            double p, y, r;
            euler_angles(p, y, r);
            return y;
        }

        [[maybe_unused]] [[nodiscard]] double euler_roll() const
        {
            double p, y, r;
            euler_angles(p, y, r);
            return r;
        }

        [[maybe_unused]] [[nodiscard]] glm::vec3 position() const
        {
            return { -translation_matrix[3][0], -translation_matrix[3][1], -translation_matrix[3][2] };
        }
    };
}

#endif //GAME_PROJ_POSITION_HPP
