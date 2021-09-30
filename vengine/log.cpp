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

static std::string ts()
{
    const size_t max_size = 64;
    std::string out;
    out.resize(max_size);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    size_t len = std::strftime(out.data(), max_size, "[%Y-%m-%d %H:%M:%S]", &now_tm);
    out.resize(len);
    return out;
}

void vengine::log::debug(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cout << ts() << DEBUG_TAG << "[" << source << "] " << message << std::endl;
    log_file() << ts() << DEBUG_TAG << "[" << source << "] " << message << std::endl;
#endif
}
void vengine::log::info(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cout << ts() << INFO_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << ts() << INFO_TAG << "[" << source << "] " << message << std::endl;
}
void vengine::log::warning(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cerr << ts() << WARNING_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << ts() << WARNING_TAG << "[" << source << "] " << message << std::endl;
}
void vengine::log::error(std::string_view source, std::string_view message)
{
#if _DEBUG
    std::cerr << ts() << ERROR_TAG << "[" << source << "] " << message << std::endl;
#endif
    log_file() << ts() << ERROR_TAG << "[" << source << "] " << message << std::endl;
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
            std::string program_data(buffer, buffer_size - 1 /* \0 */);
            std::filesystem::path p(program_data);
            p /= "vengine";
            std::filesystem::create_directories(p);
            file_path = (p / "log.txt").string();
            free(buffer);
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
