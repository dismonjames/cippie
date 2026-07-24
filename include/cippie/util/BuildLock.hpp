#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>

namespace cippie
{
    class BuildLock
    {
    public:
        BuildLock() = default;
        ~BuildLock();

        BuildLock(const BuildLock&) = delete;
        BuildLock& operator=(const BuildLock&) = delete;

        BuildLock(BuildLock&& other) noexcept;
        BuildLock& operator=(BuildLock&& other) noexcept;

        [[nodiscard]] static Result<BuildLock> acquire(
            const std::filesystem::path& projectRoot
        );

        void release() noexcept;
        [[nodiscard]] bool isAcquired() const noexcept { return m_fd != -1; }

    private:
        explicit BuildLock(int fd, std::filesystem::path lockFilePath) noexcept;

        int m_fd{-1};
        std::filesystem::path m_lockFilePath;
    };
}
