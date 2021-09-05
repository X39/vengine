//
// Created by marco.silipo on 02.09.2021.
//

#include "io.hpp"
#include "log.hpp"
#include <fstream>


size_t vengine::io::bom_skip_length(const char* begin, const char* end)
{
    if (begin == end)
    {
        return 0;
    }
    // We are comparing against unsigned
    size_t len = end - begin;
    auto unsigned_begin = reinterpret_cast<const unsigned char *>(begin);
    if (len > 3 && unsigned_begin[0] == 0xEF && unsigned_begin[1] == 0xBB && unsigned_begin[2] == 0xBF)
    {
        //UTF-8
        return 3;
    }
    else if (len > 2 && unsigned_begin[0] == 0xFE && unsigned_begin[1] == 0xFF)
    {
        //UTF-16 (BE)
        return 2;
    }
    else if (len > 2 && unsigned_begin[0] == 0xFE && unsigned_begin[1] == 0xFE)
    {
        //UTF-16 (LE)
        return 2;
    }
    else if (len > 2 && unsigned_begin[0] == 0x00 && unsigned_begin[1] == 0x00 && unsigned_begin[2] == 0xFF && unsigned_begin[3] == 0xFF)
    {
        //UTF-32 (BE)
        return 2;
    }
    else if (len > 2 && unsigned_begin[0] == 0xFF && unsigned_begin[1] == 0xFF && unsigned_begin[2] == 0x00 && unsigned_begin[3] == 0x00)
    {
        //UTF-32 (LE)
        return 2;
    }
    else if (len > 4 && unsigned_begin[0] == 0x2B
             && unsigned_begin[1] == 0x2F
             && unsigned_begin[2] == 0x76
             && (unsigned_begin[3] == 0x38 || unsigned_begin[3] == 0x39 || unsigned_begin[3] == 0x2B || unsigned_begin[3] == 0x2F))
    {
        //UTF-7
        return 4;
    }
    else if (len > 3 && unsigned_begin[0] == 0xF7 && unsigned_begin[1] == 0x64 && unsigned_begin[2] == 0x4C)
    {
        //UTF-1
        return 3;
    }
    else if (len > 3 && unsigned_begin[0] == 0xDD && unsigned_begin[1] == 0x73 && unsigned_begin[2] == 0x66 && unsigned_begin[3] == 0x73)
    {
        //UTF-EBCDIC
        return 3;
    }
    else if (len > 3 && unsigned_begin[0] == 0x0E && unsigned_begin[1] == 0xFE && unsigned_begin[2] == 0xFF)
    {
        //SCSU
        return 3;
    }
    else if (len > 4 && unsigned_begin[0] == 0xFB && unsigned_begin[1] == 0xEE && unsigned_begin[2] == 0x28)
    {
        //BOCU-1
        if (unsigned_begin[3] == 0xFF)
        {
            return 4;
        }
        return 3;
    }
    else if (len > 3 && unsigned_begin[0] == 0x84 && unsigned_begin[1] == 0x31 && unsigned_begin[2] == 0x95 && unsigned_begin[3] == 0x33)
    {
        //GB 18030
        return 3;
    }
    return 0;
}

[[maybe_unused]] bool vengine::io::read_file_from_disk(const std::filesystem::path& path, std::vector<uint8_t>& data)
{
    if (!std::filesystem::exists(path))
    {
        vengine::log::error("vengine::io::read_file_from_disk(const std::filesystem::path&, std::vector<uint8_t>&)", "File not found.");
        return false;
    }
    if (std::filesystem::is_directory(path))
    {
        vengine::log::error("vengine::io::read_file_from_disk(const std::filesystem::path&, std::vector<uint8_t>&)", "Cannot open directories.");
        return false;
    }
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        vengine::log::error("vengine::io::read_file_from_disk(const std::filesystem::path&, std::vector<uint8_t>&)", "Failed to open file: " + path.string());
        return false;
    }

    auto fileSize = file.tellg();
    data.resize(fileSize);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();
    return true;
}