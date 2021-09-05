//
// Created by marco.silipo on 05.09.2021.
//

#ifndef GAME_PROJ_LOG_HPP
#define GAME_PROJ_LOG_HPP

#include <string_view>
#include <fstream>
namespace vengine
{
    class log
    {
    private:
        static std::fstream& log_file();
    public:
        static void debug(std::string_view source, std::string_view message);
        static void info(std::string_view source, std::string_view message);
        static void warning(std::string_view source, std::string_view message);
        static void error(std::string_view source, std::string_view message);
    };
}

#endif //GAME_PROJ_LOG_HPP
