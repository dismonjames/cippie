#include <cippie/util/BuildLock.hpp>

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
    BuildLock::BuildLock(int fd, std::filesystem::path lockFilePath) noexcept
        : m_fd(fd)
        , m_lockFilePath(std::move(lockFilePath))
    {
    }

    BuildLock::~BuildLock()
    {
        release();
    }

    BuildLock::BuildLock(BuildLock&& other) noexcept
        : m_fd(other.m_fd)
        , m_lockFilePath(std::move(other.m_lockFilePath))
    {
        other.m_fd = -1;
    }

    BuildLock& BuildLock::operator=(BuildLock&& other) noexcept
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

    void BuildLock::release() noexcept
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

    Result<BuildLock> BuildLock::acquire(const std::filesystem::path& projectRoot)
    {
        std::error_code ec;
        const auto locksDir = projectRoot / ".cippie/locks";
        std::filesystem::create_directories(locksDir, ec);

        const auto lockFilePath = locksDir / "build.lock";
        auto pathStr = lockFilePath.string();

#if defined(_WIN32)
        int fd = _open(pathStr.c_str(), _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
        if (fd == -1)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open lock file: " + lockFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (_locking(fd, _LK_LOCK, 0) != 0)
        {
            _close(fd);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "another Cippie process is building this project (lock held)",
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
                .message = "failed to open lock file: " + lockFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (flock(fd, LOCK_EX | LOCK_NB) != 0)
        {
            close(fd);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "another Cippie process is building this project (lock held)",
                .location = std::nullopt,
                .notes = {}
            });
        }
#endif

        return BuildLock(fd, lockFilePath);
    }
}
