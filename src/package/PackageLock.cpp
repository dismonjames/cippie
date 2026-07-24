#include <cippie/package/PackageLock.hpp>

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <system_error>

namespace cippie
{
    PackageLock::PackageLock(int fd, std::filesystem::path lockFilePath) noexcept
        : m_fd(fd)
        , m_lockFilePath(std::move(lockFilePath))
    {
    }

    PackageLock::~PackageLock()
    {
        release();
    }

    PackageLock::PackageLock(PackageLock&& other) noexcept
        : m_fd(other.m_fd)
        , m_lockFilePath(std::move(other.m_lockFilePath))
    {
        other.m_fd = -1;
    }

    PackageLock& PackageLock::operator=(PackageLock&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_fd = other.m_fd;
            m_lockFilePath = std::move(other.m_lockFilePath);
            other.m_fd = -1;
        }
        return *this;
    }

    void PackageLock::release() noexcept
    {
        if (m_fd != -1)
        {
            flock(m_fd, LOCK_UN);
            close(m_fd);
            m_fd = -1;
        }
    }

    Result<PackageLock> PackageLock::acquire(
        const std::filesystem::path& cacheDir,
        const std::string& packageName
    )
    {
        std::error_code ec;
        const auto locksDir = cacheDir / "locks";
        std::filesystem::create_directories(locksDir, ec);

        const auto lockFilePath = locksDir / (packageName + ".lock");

        int fd = open(lockFilePath.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open package lock file: " + lockFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (flock(fd, LOCK_EX) != 0)
        {
            close(fd);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to acquire lock for package: " + packageName,
                .location = std::nullopt,
                .notes = {}
            });
        }

        return PackageLock(fd, lockFilePath);
    }
}
