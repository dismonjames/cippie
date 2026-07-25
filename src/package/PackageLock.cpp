#include <cippie/package/PackageLock.hpp>

#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/locking.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

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
#if defined(_WIN32)
            _locking(m_fd, _LK_UNLCK, 0);
            _close(m_fd);
#else
            flock(m_fd, LOCK_UN);
            close(m_fd);
#endif
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
        auto pathStr = lockFilePath.string();

#if defined(_WIN32)
        int fd = _open(pathStr.c_str(), _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
        if (fd == -1)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open package lock file: " + lockFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (_locking(fd, _LK_LOCK, 0) != 0)
        {
            _close(fd);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to acquire lock for package: " + packageName,
                .location = std::nullopt,
                .notes = {}
            });
        }
#else
        int fd = open(pathStr.c_str(), O_RDWR | O_CREAT, 0666);
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
#endif

        return PackageLock(fd, lockFilePath);
    }
}
