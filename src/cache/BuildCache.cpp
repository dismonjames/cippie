#include <cippie/cache/BuildCache.hpp>

namespace cippie
{
    bool BuildCache::isUpToDate(
        const std::filesystem::path& source,
        const std::filesystem::path& object
    ) const
    {
        std::error_code error;

        if (!std::filesystem::is_regular_file(object, error))
        {
            return false;
        }

        const auto sourceTime =
            std::filesystem::last_write_time(source, error);

        if (error)
        {
            return false;
        }

        const auto objectTime =
            std::filesystem::last_write_time(object, error);

        if (error)
        {
            return false;
        }

        return objectTime >= sourceTime;
    }
}
