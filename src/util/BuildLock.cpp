#include <cippie/util/BuildLock.hpp>

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <system_error>

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
            flock(m_fd, LOCK_UN);
            close(m_fd);
            m_fd = -1;
        }
    }

    Result<BuildLock> BuildLock::acquire(const std::filesystem::path& projectRoot)
    {
        std::error_code ec;
        const auto locksDir = projectRoot / ".cippie/locks";
        std::filesystem::create_directories(locksDir, ec);

        const auto lockFilePath = locksDir / "build.lock";

        int fd = open(lockFilePath.c_str(), O_RDWR | O_CREAT, 0666);
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

        return BuildLock(fd, lockFilePath);
    }
}
