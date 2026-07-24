#include <cippie/cache/DepFileParser.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cippie
{
    std::vector<std::filesystem::path> DepFileParser::parseString(const std::string& content)
    {
        std::vector<std::filesystem::path> dependencies;

        // Join line continuations (\ + \n)
        std::string unescapedContent;
        unescapedContent.reserve(content.size());

        for (size_t i = 0; i < content.size(); ++i)
        {
            if (content[i] == '\\' && i + 1 < content.size() && (content[i + 1] == '\n' || content[i + 1] == '\r'))
            {
                if (content[i + 1] == '\r' && i + 2 < content.size() && content[i + 2] == '\n')
                {
                    i += 2;
                }
                else
                {
                    i += 1;
                }
                unescapedContent.push_back(' ');
                continue;
            }
            unescapedContent.push_back(content[i]);
        }

        std::stringstream ss(unescapedContent);
        std::string line;

        while (std::getline(ss, line))
        {
            if (line.empty()) continue;

            auto colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string rightSide = line.substr(colonPos + 1);
            std::string currentPath;
            bool escaped = false;

            for (size_t i = 0; i < rightSide.size(); ++i)
            {
                char c = rightSide[i];

                if (escaped)
                {
                    currentPath.push_back(c);
                    escaped = false;
                }
                else if (c == '\\')
                {
                    if (i + 1 < rightSide.size() && (rightSide[i + 1] == ' ' || rightSide[i + 1] == '\\' || rightSide[i + 1] == '#' || rightSide[i + 1] == ':'))
                    {
                        escaped = true;
                    }
                    else
                    {
                        currentPath.push_back(c);
                    }
                }
                else if (std::isspace(static_cast<unsigned char>(c)))
                {
                    if (!currentPath.empty())
                    {
                        dependencies.push_back(std::filesystem::path(currentPath).lexically_normal());
                        currentPath.clear();
                    }
                }
                else
                {
                    currentPath.push_back(c);
                }
            }

            if (!currentPath.empty())
            {
                dependencies.push_back(std::filesystem::path(currentPath).lexically_normal());
            }
        }

        std::sort(dependencies.begin(), dependencies.end());
        dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());

        return dependencies;
    }

    Result<std::vector<std::filesystem::path>> DepFileParser::parse(
        const std::filesystem::path& depFilePath
    ) const
    {
        std::ifstream file(depFilePath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open dependency file: " + depFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return parseString(content);
    }
}
