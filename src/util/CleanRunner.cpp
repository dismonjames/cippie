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

        // Must be non-empty and inside project root under .cippie
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

        const std::string targetStr = canonicalTarget.string();
        const std::string expectedSub = (canonicalRoot / ".cippie").string();

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

    Result<void> CleanRunner::clean(
        const std::filesystem::path& projectRoot,
        bool cacheOnly,
        bool cleanAll
    ) const
    {
        std::error_code ec;

        if (cleanAll)
        {
            const auto targetDir = projectRoot / ".cippie";
            if (!isSafeToDelete(targetDir, projectRoot))
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "unsafe deletion target: " + targetDir.string(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            if (std::filesystem::exists(targetDir, ec))
            {
                std::filesystem::remove_all(targetDir, ec);
                if (ec)
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::fileReadFailed,
                        .message = "failed to clean directory: " + ec.message(),
                        .location = std::nullopt,
                        .notes = {}
                    });
                }
            }
            logger_.info("Removed .cippie");
            return {};
        }

        if (cacheOnly)
        {
            const auto cacheDir = projectRoot / ".cippie/cache";
            const auto manifestsDir = projectRoot / ".cippie/manifests";

            if (std::filesystem::exists(cacheDir, ec)) std::filesystem::remove_all(cacheDir, ec);
            if (std::filesystem::exists(manifestsDir, ec)) std::filesystem::remove_all(manifestsDir, ec);

            logger_.info("Removed .cippie cache and manifests");
            return {};
        }

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
