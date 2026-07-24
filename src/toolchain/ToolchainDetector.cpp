#include <cippie/toolchain/ToolchainDetector.hpp>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

namespace cippie
{
    namespace
    {
        std::optional<std::filesystem::path> findInPath(const std::string& candidate)
        {
            if (candidate.empty())
            {
                return std::nullopt;
            }

            if (candidate.find('/') != std::string::npos)
            {
                if (std::filesystem::exists(candidate))
                {
                    return std::filesystem::absolute(candidate);
                }
                return std::nullopt;
            }

            const char* pathEnv = std::getenv("PATH");
            if (!pathEnv)
            {
                return std::nullopt;
            }

            std::stringstream ss(pathEnv);
            std::string dir;
            while (std::getline(ss, dir, ':'))
            {
                if (dir.empty())
                {
                    continue;
                }
                auto fullPath = std::filesystem::path(dir) / candidate;
                std::error_code ec;
                if (std::filesystem::exists(fullPath, ec) &&
                    std::filesystem::is_regular_file(fullPath, ec))
                {
                    return std::filesystem::absolute(fullPath);
                }
            }

            return std::nullopt;
        }

        std::string getCompilerVersion(const std::filesystem::path& compilerPath)
        {
            std::string cmd = compilerPath.string() + " --version 2>&1";
            FILE* pipe = popen(cmd.c_str(), "r");
            if (!pipe)
            {
                return "unknown";
            }

            std::array<char, 256> buffer;
            std::string result;
            if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
            {
                result = buffer.data();
                while (!result.empty() &&
                       (result.back() == '\n' || result.back() == '\r'))
                {
                    result.pop_back();
                }
            }
            pclose(pipe);
            return result.empty() ? "unknown" : result;
        }
    }

    Toolchain ToolchainDetector::detect() const
    {
        Toolchain toolchain;
        toolchain.host = TargetTriple::detectHost();
        toolchain.target = toolchain.host;

        std::vector<std::string> candidates;

        if (const char* cxxEnv = std::getenv("CXX"))
        {
            if (*cxxEnv != '\0')
            {
                candidates.push_back(cxxEnv);
            }
        }

        candidates.push_back("clang++");
        candidates.push_back("g++");
        candidates.push_back("c++");

        for (const auto& candidate : candidates)
        {
            auto found = findInPath(candidate);
            if (found.has_value())
            {
                toolchain.cxxCompiler = *found;
                toolchain.linker = *found;

                const std::string nameStr = candidate;
                if (nameStr.find("clang") != std::string::npos)
                {
                    toolchain.name = "clang";
                    toolchain.compilerFamily = CompilerFamily::clang;
                }
                else if (nameStr.find("g++") != std::string::npos || nameStr.find("gcc") != std::string::npos)
                {
                    toolchain.name = "gcc";
                    toolchain.compilerFamily = CompilerFamily::gcc;
                }
                else
                {
                    toolchain.name = "host";
                    toolchain.compilerFamily = CompilerFamily::gcc;
                }

                auto archiverFound = findInPath("ar");
                if (archiverFound.has_value())
                {
                    toolchain.archiver = *archiverFound;
                }

                toolchain.version = getCompilerVersion(*found);
                return toolchain;
            }
        }

        return toolchain;
    }
}
