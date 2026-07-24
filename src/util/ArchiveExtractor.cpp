#include <cippie/util/ArchiveExtractor.hpp>
#include <cippie/process/Process.hpp>

#include <system_error>

namespace cippie
{
    bool ArchiveExtractor::isPathSafe(
        const std::filesystem::path& entryPath,
        const std::filesystem::path& extractionRoot
    ) noexcept
    {
        std::error_code ec;
        auto canonicalRoot = std::filesystem::weakly_canonical(extractionRoot, ec);
        if (ec) return false;

        auto canonicalEntry = std::filesystem::weakly_canonical(entryPath, ec);
        if (ec) return false;

        const std::string rootStr = canonicalRoot.string();
        const std::string entryStr = canonicalEntry.string();

        if (entryStr.rfind(rootStr, 0) != 0)
        {
            return false;
        }

        if (std::filesystem::is_symlink(entryPath, ec))
        {
            auto target = std::filesystem::read_symlink(entryPath, ec);
            if (!ec)
            {
                auto canonicalTarget = std::filesystem::weakly_canonical(target, ec);
                if (!ec && canonicalTarget.string().rfind(rootStr, 0) != 0)
                {
                    return false;
                }
            }
        }

        return true;
    }

    Result<void> ArchiveExtractor::extract(
        const std::filesystem::path& archivePath,
        const std::filesystem::path& destinationDirectory
    )
    {
        std::error_code ec;
        std::filesystem::create_directories(destinationDirectory.parent_path(), ec);

        const auto tmpDir = destinationDirectory.string() + ".tmp-extract";
        std::filesystem::remove_all(tmpDir, ec);
        std::filesystem::create_directories(tmpDir, ec);

        ProcessRequest req;
        req.executable = "tar";
        req.arguments = {"-xzf", archivePath.string(), "-C", tmpDir};
        req.captureOutput = true;

        Process process;
        auto res = process.run(req);

        if (res.exitCode != 0)
        {
            std::filesystem::remove_all(tmpDir, ec);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to extract archive: " + res.stderrOutput,
                .location = std::nullopt,
                .notes = {}
            });
        }

        // Validate all extracted files
        for (auto const& entry : std::filesystem::recursive_directory_iterator(tmpDir, ec))
        {
            if (!isPathSafe(entry.path(), tmpDir))
            {
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "archive contains malicious path escaping extraction root: " + entry.path().string(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }

        // Handle single root subdirectory in archive if present (e.g. fmt-11.2.0/)
        std::filesystem::path sourceToMove = tmpDir;
        size_t entryCount = 0;
        std::filesystem::path singleSubdir;

        for (auto const& entry : std::filesystem::directory_iterator(tmpDir, ec))
        {
            entryCount++;
            if (entry.is_directory())
            {
                singleSubdir = entry.path();
            }
        }

        if (entryCount == 1 && !singleSubdir.empty())
        {
            sourceToMove = singleSubdir;
        }

        std::filesystem::remove_all(destinationDirectory, ec);
        std::filesystem::rename(sourceToMove, destinationDirectory, ec);

        if (ec)
        {
            // Fallback to copy if rename across filesystems
            std::filesystem::copy(sourceToMove, destinationDirectory, std::filesystem::copy_options::recursive, ec);
            std::filesystem::remove_all(tmpDir, ec);
            if (ec)
            {
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to move extracted archive into place: " + ec.message(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }
        else
        {
            std::filesystem::remove_all(tmpDir, ec);
        }

        return {};
    }
}
