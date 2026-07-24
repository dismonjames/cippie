#include <cippie/config/ConfigLoader.hpp>

#include <stdexcept>
#include <utility>

namespace cippie
{
    Project ConfigLoader::load(
        const std::filesystem::path& projectRoot
    ) const
    {
        const auto configurationFile = projectRoot / "Cippiefile";

        if (!std::filesystem::is_regular_file(configurationFile))
        {
            throw std::runtime_error(
                "Cippiefile does not exist in project root"
            );
        }

        Project project;
        project.name = projectRoot.filename().string();
        project.cppStandard = 23;
        project.rootDirectory = projectRoot;
        project.configurationFile = configurationFile;

        Target target;
        target.name = project.name;
        target.type = TargetType::executable;
        target.entry = projectRoot / "src/main.cpp";
        target.includeDirectories.push_back(projectRoot / "include");

        project.targets.push_back(std::move(target));
        return project;
    }
}
