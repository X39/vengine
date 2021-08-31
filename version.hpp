#pragma once
#include <string>
#include <utility>

namespace x39
{
    struct version
    {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;
        std::string additional;

        [[maybe_unused]] version(uint32_t ma, uint32_t mi, uint32_t pa) :
            major(ma),
            minor(mi),
            patch(pa),
            additional() {}

        [[maybe_unused]] version(uint32_t ma, uint32_t mi, uint32_t pa, std::string add) :
            major(ma),
            minor(mi),
            patch(pa),
            additional(std::move(add)) {}

        [[maybe_unused]] [[nodiscard]] std::string to_string() const
        {
            std::string s_ma = std::to_string(major);
            std::string s_mi = std::to_string(minor);
            std::string s_pa = std::to_string(patch);

            std::string out;
            if (additional.empty())
            {
                out.reserve(
                    s_ma.length() +
                    1 +
                    s_mi.length() +
                    1 +
                    s_pa.length()
                );
                out.append(s_ma);
                out.append(1, '.');
                out.append(s_mi);
                out.append(1, '.');
                out.append(s_pa);
            }
            else
            {
                out.reserve(
                    s_ma.length() +
                    1 +
                    s_mi.length() +
                    1 +
                    s_pa.length() +
                    3 +
                    additional.length()
                );
                out.append(s_ma);
                out.append(1, '.');
                out.append(s_mi);
                out.append(1, '.');
                out.append(s_pa);
                out.append(1, ' ');
                out.append(1, '-');
                out.append(1, ' ');
                out.append(additional);
            }
            return out;
        }
    };
}