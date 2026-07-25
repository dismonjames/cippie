#include <cippie/project/ProjectGenerator.hpp>

#include <cctype>
#include <fstream>

namespace cippie
{
    bool ProjectGenerator::isValidProjectName(std::string_view name) noexcept
    {
        if (name.empty())
        {
            return false;
        }

        const char first = name.front();
        if (!std::isalpha(static_cast<unsigned char>(first)) && first != '_')
        {
            return false;
        }

        for (const char c : name)
        {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            {
                return false;
            }
        }

        return true;
    }

    Result<std::filesystem::path> ProjectGenerator::generate(
        std::string_view projectName,
        const std::filesystem::path& parentDirectory
    ) const
    {
        if (!isValidProjectName(projectName))
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "invalid project name '" + std::string(projectName) + "'",
                .location = std::nullopt,
                .notes = {}
            });
        }

        const auto projectDir = parentDirectory / std::string(projectName);
        std::error_code ec;

        if (std::filesystem::exists(projectDir, ec))
        {
            if (std::filesystem::is_directory(projectDir, ec))
            {
                std::filesystem::directory_iterator iter(projectDir, ec);
                if (!ec && iter != std::filesystem::directory_iterator())
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::validationFailed,
                        .message = "directory '" + std::string(projectName) + "' already exists and is not empty",
                        .location = std::nullopt,
                        .notes = {}
                    });
                }
            }
        }

        std::filesystem::create_directories(projectDir / "src", ec);
        std::filesystem::create_directories(projectDir / "include", ec);
        std::filesystem::create_directories(projectDir / "tests", ec);

        // Cippiefile
        std::ofstream cippiefile(projectDir / "Cippiefile");
        cippiefile << "project(\"" << projectName << "\") {\n"
                   << "    cpp = 23;\n\n"
                   << "    executable(\"" << projectName << "\") {\n"
                   << "        entry = \"src/main.cpp\";\n"
                   << "        sources = [\"src/**/*.cpp\"];\n"
                   << "        includes = [\"include\"];\n"
                   << "        dependencies = [];\n"
                   << "    }\n"
                   << "}\n";
        cippiefile.close();

        // main.cpp
        std::ofstream mainCpp(projectDir / "src/main.cpp");
        mainCpp << "#include <iostream>\n\n"
                << "int main()\n"
                << "{\n"
                << "    std::cout << \"Hello from " << projectName << "!\\n\";\n"
                << "    return 0;\n"
                << "}\n";
        mainCpp.close();

        // .gitignore
        std::ofstream gitignore(projectDir / ".gitignore");
        gitignore << ".cippie/\nbuild/\n";
        gitignore.close();

        // README.md
        std::ofstream readme(projectDir / "README.md");
        readme << "# " << projectName << "\n\nBuilt with Cippie v0.1.0.\n\n"
               << "## Building and Running\n\n"
               << "```bash\n"
               << "# Build the project\n"
               << "cippie build\n\n"
               << "# Run the default target\n"
               << "cippie run\n\n"
               << "# Run project tests\n"
               << "cippie test\n"
               << "```\n";
        readme.close();

        return projectDir;
    }
}
