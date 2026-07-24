#include <cippie/config/CippiefileEditor.hpp>

#include <fstream>
#include <sstream>
#include <system_error>

namespace cippie
{
    Result<void> CippiefileEditor::addDependency(
        const std::filesystem::path& cippiefilePath,
        const std::string& packageExpr
    )
    {
        std::ifstream file(cippiefilePath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open Cippiefile: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        size_t depPos = content.find("dependencies = [");
        if (depPos == std::string::npos)
        {
            depPos = content.find("dependencies=");
        }

        if (depPos != std::string::npos)
        {
            size_t insertPos = content.find('[', depPos);
            if (insertPos != std::string::npos)
            {
                std::string newEntry = "\n        " + packageExpr + ",";
                content.insert(insertPos + 1, newEntry);
            }
        }
        else
        {
            size_t projEnd = content.rfind('}');
            if (projEnd != std::string::npos)
            {
                std::string newBlock = "    dependencies = [\n        " + packageExpr + "\n    ];\n";
                content.insert(projEnd, newBlock);
            }
            else
            {
                content += "\ndependencies = [\n    " + packageExpr + "\n];\n";
            }
        }

        std::ofstream outFile(cippiefilePath);
        if (!outFile.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to write Cippiefile: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        outFile << content;
        return {};
    }

    Result<void> CippiefileEditor::removeDependency(
        const std::filesystem::path& cippiefilePath,
        const std::string& packageName
    )
    {
        std::ifstream file(cippiefilePath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open Cippiefile: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::string line;
        std::ostringstream out;

        while (std::getline(file, line))
        {
            if (line.find("\"" + packageName + "\"") != std::string::npos &&
                (line.find("package(") != std::string::npos ||
                 line.find("pathPackage(") != std::string::npos ||
                 line.find("gitPackage(") != std::string::npos))
            {
                continue; // Skip line removing dependency
            }
            out << line << "\n";
        }

        file.close();

        std::ofstream outFile(cippiefilePath);
        if (!outFile.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to write Cippiefile: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        outFile << out.str();
        return {};
    }
}
