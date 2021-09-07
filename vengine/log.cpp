//
// Created by marco.silipo on 05.09.2021.
//

#include "log.hpp"

#include <iostream>
#include <filesystem>
#include <cstdlib>

#define ERROR_TAG   "[ERR]"
#define WARNING_TAG "[WRN]"
#define INFO_TAG    "[INF]"
#define DEBUG_TAG   "[DBG]"
#define NEW_LINE "\r\n"

void vengine::log::debug(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cout << DEBUG_TAG << "[" << source << "] " << message << std::endl;
    log_file() << DEBUG_TAG << "[" << source << "] " << message << std::endl;
#endif
}
void vengine::log::info(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cout << INFO_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << INFO_TAG << "[" << source << "] " << message << std::endl;
}
void vengine::log::warning(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cerr << WARNING_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << WARNING_TAG << "[" << source << "] " << message << std::endl;
}
void vengine::log::error(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cerr << ERROR_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << ERROR_TAG << "[" << source << "] " << message << std::endl;
}

std::fstream& vengine::log::log_file()
{
    static std::fstream file;
    if (!file.good() || !file.is_open())
    {
        std::string file_path;
#if WIN32
        size_t buffer_size;
        char* buffer;
        if (_dupenv_s(&buffer, &buffer_size, "programdata") != -1)
        {
            std::string program_data(buffer, buffer_size);
            free(buffer);
            std::filesystem::path p(program_data);
            p /= "vengine";
            std::filesystem::create_directories(p);
            file_path = (p / "log.txt").string();
        }
        else
        {
            file_path = "log.txt";
        }
#else
        file_path = "log.txt";
#endif
        file.open(file_path, std::ios::out | std::ios::in | std::ios::app);
        if (!file.good()) {
            abort();
        }
    }
    return file;
}
