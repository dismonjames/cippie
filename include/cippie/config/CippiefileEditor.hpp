#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    class CippiefileEditor
    {
    public:
        CippiefileEditor() = default;

        [[nodiscard]] static Result<void> addDependency(
            const std::filesystem::path& cippiefilePath,
            const std::string& packageExpr
        );

        [[nodiscard]] static Result<void> removeDependency(
            const std::filesystem::path& cippiefilePath,
            const std::string& packageName
        );
    };
}
