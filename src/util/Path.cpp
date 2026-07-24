#include <cippie/util/Path.hpp>

namespace cippie
{
    std::filesystem::path normalizePath(
        const std::filesystem::path& path
    )
    {
        std::error_code error;
        const auto absolute = std::filesystem::absolute(path, error);

        if (error)
        {
            return path.lexically_normal();
        }

        return absolute.lexically_normal();
    }
}
