#pragma once

#include <cstddef>
#include <filesystem>

namespace cippie
{
    struct SourceLocation
    {
        std::filesystem::path file;
        std::size_t line{1};
        std::size_t column{1};
        std::size_t offset{0};
    };
}
