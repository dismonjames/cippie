#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/diagnostics/Logger.hpp>

#include <filesystem>

namespace cippie
{
    class CleanRunner
    {
    public:
        explicit CleanRunner(const Logger& logger);

        [[nodiscard]] static bool isSafeToDelete(
            const std::filesystem::path& targetPath,
            const std::filesystem::path& projectRoot
        ) noexcept;

        [[nodiscard]] Result<void> clean(
            const std::filesystem::path& projectRoot,
            bool cacheOnly = false,
            bool cleanAll = false
        ) const;

    private:
        const Logger& logger_;
    };
}
