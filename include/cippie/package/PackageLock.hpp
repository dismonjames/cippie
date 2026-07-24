#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    class PackageLock
    {
    public:
        PackageLock() = default;
        ~PackageLock();

        PackageLock(const PackageLock&) = delete;
        PackageLock& operator=(const PackageLock&) = delete;

        PackageLock(PackageLock&& other) noexcept;
        PackageLock& operator=(PackageLock&& other) noexcept;

        [[nodiscard]] static Result<PackageLock> acquire(
            const std::filesystem::path& cacheDir,
            const std::string& packageName
        );

        void release() noexcept;

    private:
        explicit PackageLock(int fd, std::filesystem::path lockFilePath) noexcept;

        int m_fd{-1};
        std::filesystem::path m_lockFilePath;
    };
}
