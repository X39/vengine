//
// Created by marco.silipo on 02.09.2021.
//

#ifndef GAME_PROJ_IO_HPP
#define GAME_PROJ_IO_HPP
#include <vector>
#include <array>
#include <filesystem>
#include <string_view>
namespace vengine::io
{
    [[maybe_unused]] size_t bom_skip_length(const char* begin, const char* end);
    [[maybe_unused]] inline size_t bom_skip_length(std::vector<char> data) { return bom_skip_length(data.data(), data.data() + data.size()); }
    template<size_t size>
    [[maybe_unused]] inline size_t bom_skip_length(std::array<char, size> data) { return bom_skip_length(data.data(), size); }

    [[maybe_unused]] bool read_file_from_disk(const std::filesystem::path& path, std::vector<uint8_t>& in_data);
}

#endif //GAME_PROJ_IO_HPP
