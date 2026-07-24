#include <cippie/util/CleanRunner.hpp>

#include <cstdlib>
#include <system_error>

namespace cippie
{
    CleanRunner::CleanRunner(const Logger& logger)
        : logger_(logger)
    {
    }

    bool CleanRunner::isSafeToDelete(
        const std::filesystem::path& targetPath,
        const std::filesystem::path& projectRoot
    ) noexcept
    {
        std::error_code ec;
        auto canonicalTarget = std::filesystem::weakly_canonical(targetPath, ec);
        if (ec)
        {
            return false;
        }

        auto canonicalRoot = std::filesystem::weakly_canonical(projectRoot, ec);
        if (ec)
        {
            return false;
        }

        // Must be non-empty and inside project root under .cippie/build
        if (canonicalTarget.empty() || canonicalTarget == "/" || canonicalTarget == canonicalRoot)
        {
            return false;
        }

        const char* homeEnv = std::getenv("HOME");
        if (homeEnv)
        {
            auto canonicalHome = std::filesystem::weakly_canonical(homeEnv, ec);
            if (!ec && (canonicalTarget == canonicalHome || canonicalTarget == "/home"))
            {
                return false;
            }
        }

        // Ensure canonicalTarget contains .cippie/build
        const std::string targetStr = canonicalTarget.string();
        const std::string expectedSub = (canonicalRoot / ".cippie/build").string();

        if (targetStr.rfind(expectedSub, 0) != 0 && targetStr != expectedSub)
        {
            return false;
        }

        // Check symlinks
        if (std::filesystem::is_symlink(targetPath, ec))
        {
            auto linkTarget = std::filesystem::read_symlink(targetPath, ec);
            if (!ec)
            {
                auto canonicalLink = std::filesystem::weakly_canonical(linkTarget, ec);
                if (canonicalLink.string().rfind(canonicalRoot.string(), 0) != 0)
                {
                    return false;
                }
            }
        }

        return true;
    }

    Result<void> CleanRunner::clean(const std::filesystem::path& projectRoot) const
    {
        const auto buildDirectory = projectRoot / ".cippie/build";

        if (!isSafeToDelete(buildDirectory, projectRoot))
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "unsafe deletion target: " + buildDirectory.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::error_code ec;
        if (std::filesystem::exists(buildDirectory, ec))
        {
            std::filesystem::remove_all(buildDirectory, ec);
            if (ec)
            {
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to clean build directory: " + ec.message(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }

        logger_.info("Removed .cippie/build");
        return {};
    }
}
