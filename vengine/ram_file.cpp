//
// Created by marco.silipo on 02.09.2021.
//

#include "ram_file.hpp"
#include "io.hpp"
#include "log.hpp"

std::optional<vengine::ram_file> vengine::ram_file::from_disk(const std::filesystem::path& path)
{
    std::vector<uint8_t> data;
    if (!vengine::io::read_file_from_disk(path, data))
    {
        return {};
    }
    return vengine::ram_file(data.begin(), data.end());
}
