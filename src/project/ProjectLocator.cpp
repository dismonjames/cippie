#include <cippie/project/ProjectLocator.hpp>

namespace cippie
{
    std::optional<std::filesystem::path> ProjectLocator::locate(
        const std::filesystem::path& startDirectory
    ) const
    {
        std::error_code error;
        std::filesystem::path current =
            std::filesystem::absolute(startDirectory, error);

        if (error)
        {
            return std::nullopt;
        }

        while (true)
        {
            const auto configurationFile = current / "Cippiefile";

            if (std::filesystem::is_regular_file(configurationFile, error))
            {
                return current;
            }

            error.clear();
            const auto parent = current.parent_path();

            if (parent.empty() || parent == current)
            {
                break;
            }

            current = parent;
        }

        return std::nullopt;
    }
}
