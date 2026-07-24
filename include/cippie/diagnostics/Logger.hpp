#pragma once

#include <cstddef>
#include <string_view>

namespace cippie
{
    class Logger
    {
    public:
        void info(std::string_view message) const;
        void warning(std::string_view message) const;
        void error(std::string_view message) const;

        void buildStep(
            std::size_t current,
            std::size_t total,
            std::string_view action,
            std::string_view subject
        ) const;
    };
}
