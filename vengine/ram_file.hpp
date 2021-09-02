//
// Created by marco.silipo on 02.09.2021.
//

#ifndef GAME_PROJ_RAM_FILE_HPP
#define GAME_PROJ_RAM_FILE_HPP

#include <cstdint>
#include <vector>
#include <filesystem>
#include <optional>

namespace vengine
{
    class ram_file
    {
    public:
    private:
        std::vector<uint8_t> m_data;
    public:
        template<typename T>
        [[maybe_unused]] ram_file(T begin, T end)
        {
            auto size = end - begin;
            m_data.resize(size);
            std::copy(begin, end, m_data.begin());
        }
        [[maybe_unused]] [[nodiscard]] const uint8_t* data() const { return m_data.data(); }
        [[maybe_unused]] [[nodiscard]] size_t size() const { return m_data.size(); }


        [[maybe_unused]] [[nodiscard]] const uint8_t* begin() const { return m_data.data(); }
        [[maybe_unused]] [[nodiscard]] const uint8_t* end() const { return m_data.data() + m_data.size(); }


        [[maybe_unused]] static std::optional<vengine::ram_file> from_disk(const std::filesystem::path& path);
    };
}

#endif //GAME_PROJ_RAM_FILE_HPP
