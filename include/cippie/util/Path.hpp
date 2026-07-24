#pragma once

#include <filesystem>

namespace cippie
{
    [[nodiscard]] std::filesystem::path normalizePath(
        const std::filesystem::path& path
    );
}
