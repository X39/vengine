//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_RESULT_HPP
#define GAME_PROJ_RESULT_HPP

#include <vulkan/vulkan.h>

#include <optional>

namespace vengine::vulkan_utils
{
    template<typename T>
    class result
    {
        VkResult m_result;
        std::optional<T> m_payload;
    public:
        result(VkResult result) : m_result(result), m_payload({}) {}
        [[maybe_unused]] [[nodiscard]] bool has_value() const { return m_payload.has_value(); }
        [[maybe_unused]] [[nodiscard]] VkResult result() const { return m_result; }
        [[maybe_unused]] [[nodiscard]] T value() const { return m_payload.value(); }
    };
}

#endif //GAME_PROJ_RESULT_HPP
