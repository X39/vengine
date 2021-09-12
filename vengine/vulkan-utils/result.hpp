//
// Created by marco.silipo on 03.09.2021.
//

#ifndef GAME_PROJ_RESULT_HPP
#define GAME_PROJ_RESULT_HPP

#include <vulkan/vulkan.h>

#include <optional>
#include <utility>
#include <string>
#include <string_view>

namespace vengine::vulkan_utils
{
    template<typename T>
    class result
    {
        VkResult m_result;
        std::string m_message;
        std::optional<T> m_payload;
    public:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
        [[maybe_unused]] result(VkResult result) : m_result(result), m_payload({}) {}
        [[maybe_unused]] result(VkResult result, std::string message) : m_result(result), m_payload({}), m_message(std::move(message)) {}
        [[maybe_unused]] result(VkResult result, std::string_view message) : m_result(result), m_payload({}), m_message(std::string(message.begin(), message.end())) {}
        [[maybe_unused]] result(std::string message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_payload({}), m_message(std::move(message)) {}
        [[maybe_unused]] result(std::string_view message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_payload({}), m_message(std::string(message.begin(), message.end())) {}
        [[maybe_unused]] result(const char* message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_payload({}), m_message(message) {}
        [[maybe_unused]] result(T payload) : m_result(VK_SUCCESS), m_payload(std::move(payload)) {}
#pragma clang diagnostic pop
        [[maybe_unused]] result(VkResult result, T payload) : m_result(result), m_payload(std::move(payload)) {}


        [[maybe_unused]] [[nodiscard]] bool has_value() const { return m_payload.has_value(); }
        [[maybe_unused]] [[nodiscard]] VkResult vk_result() const { return m_result; }
        [[maybe_unused]] [[nodiscard]] T value() const { return m_payload.value(); }
        [[maybe_unused]] [[nodiscard]] std::string_view message() const { return m_message; }
        [[maybe_unused]] [[nodiscard]] bool good() const { return m_result == VK_SUCCESS; }

        operator bool() const { return good(); } // NOLINT(google-explicit-constructor)
    };
    template<>
    class result<void>
    {
        VkResult m_result;
        std::string m_message;
    public:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
        [[maybe_unused]] result() : m_result(VkResult::VK_SUCCESS) {}
        [[maybe_unused]] result(VkResult result) : m_result(result) {}
        [[maybe_unused]] result(VkResult result, std::string message) : m_result(result), m_message(std::move(message)) {}
        [[maybe_unused]] result(VkResult result, std::string_view message) : m_result(result), m_message(std::string(message.begin(), message.end())) {}
        [[maybe_unused]] result(std::string message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_message(std::move(message)) {}
        [[maybe_unused]] result(std::string_view message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_message(std::string(message.begin(), message.end())) {}
        [[maybe_unused]] result(const char* message) : m_result(VkResult::VK_ERROR_UNKNOWN), m_message(message) {}
#pragma clang diagnostic pop


        [[maybe_unused]] [[nodiscard]] VkResult vk_result() const { return m_result; }
        [[maybe_unused]] [[nodiscard]] std::string_view message() const { return m_message; }
        [[maybe_unused]] [[nodiscard]] bool good() const { return m_result == VK_SUCCESS; }

        operator bool() const { return good(); } // NOLINT(google-explicit-constructor)
    };

}

#endif //GAME_PROJ_RESULT_HPP
