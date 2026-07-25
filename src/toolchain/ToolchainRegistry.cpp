#include <cippie/toolchain/ToolchainRegistry.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace cippie
{
    std::filesystem::path ToolchainRegistry::getConfigDir()
    {
        const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
        if (xdgConfig != nullptr && xdgConfig[0] != '\0')
        {
            return std::filesystem::path(xdgConfig) / "cippie" / "toolchains";
        }

        const char* home = std::getenv("HOME");
        if (home != nullptr && home[0] != '\0')
        {
            return std::filesystem::path(home) / ".config" / "cippie" / "toolchains";
        }

        return std::filesystem::temp_directory_path() / "cippie" / "toolchains";
    }

    void ToolchainRegistry::addToolchain(Toolchain t)
    {
        m_toolchains.push_back(std::move(t));
    }

    Result<ToolchainRegistry> ToolchainRegistry::load(const std::filesystem::path& configDir)
    {
        ToolchainRegistry registry;

        std::error_code ec;
        if (!std::filesystem::is_directory(configDir, ec))
        {
            return registry; // Empty registry if dir doesn't exist
        }

        std::filesystem::directory_iterator iter(configDir, ec);
        if (ec)
        {
            return registry;
        }

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(configDir, ec))
        {
            if (ec) { ec.clear(); continue; }
            if (entry.path().extension() == ".tc")
            {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end()); // Deterministic order

        for (const auto& filePath : files)
        {
            std::ifstream file(filePath);
            if (!file.is_open()) continue;

            Toolchain tc;
            tc.host = TargetTriple::detectHost();
            tc.target = tc.host;

            std::string line;
            while (std::getline(file, line))
            {
                // Skip comments and empty lines
                if (line.empty() || line.front() == '#') continue;

                auto eqPos = line.find('=');
                if (eqPos == std::string::npos) continue;

                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                // Trim whitespace
                auto ltrim = [](std::string& s) {
                    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
                };
                auto rtrim = [](std::string& s) {
                    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
                };

                ltrim(key); rtrim(key);
                ltrim(value); rtrim(value);

                if (key == "name") tc.name = value;
                else if (key == "cxx") tc.cxxCompiler = value;
                else if (key == "cc") tc.cCompiler = value;
                else if (key == "ar") tc.archiver = value;
                else if (key == "linker") tc.linker = value;
                else if (key == "sysroot") tc.sysroot = value;
                else if (key == "compiler_family")
                {
                    if (value == "clang") tc.compilerFamily = CompilerFamily::clang;
                    else if (value == "gcc") tc.compilerFamily = CompilerFamily::gcc;
                    else if (value == "msvc") tc.compilerFamily = CompilerFamily::msvc;
                }
                else if (key == "target")
                {
                    auto tripleRes = TargetTriple::parse(value);
                    if (tripleRes.has_value()) tc.target = *tripleRes;
                }
            }

            // Use cxx as linker if linker not set
            if (tc.linker.empty() || tc.linker == "c++")
            {
                tc.linker = tc.cxxCompiler;
            }

            if (!tc.name.empty() && !tc.cxxCompiler.empty())
            {
                registry.addToolchain(std::move(tc));
            }
        }

        return registry;
    }

    Result<Toolchain> ToolchainRegistry::findForTarget(const TargetTriple& target) const
    {
        std::vector<const Toolchain*> matches;
        for (const auto& tc : m_toolchains)
        {
            if (tc.target == target)
            {
                matches.push_back(&tc);
            }
        }

        if (matches.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::compilerNotFound,
                .message = "no registered toolchain found for target '" + target.toString() + "'",
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (matches.size() > 1)
        {
            std::string msg = "ambiguous toolchain for target '" + target.toString() + "'; multiple registered toolchains match:";
            for (const auto* tc : matches)
            {
                msg += "\n  " + tc->name;
            }
            msg += "\nUse --toolchain <name> to select one explicitly.";
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = msg,
                .location = std::nullopt,
                .notes = {}
            });
        }

        return *matches[0];
    }

    Result<Toolchain> ToolchainRegistry::findByName(const std::string& name) const
    {
        for (const auto& tc : m_toolchains)
        {
            if (tc.name == name) return tc;
        }

        return std::unexpected(Error{
            .code = ErrorCode::compilerNotFound,
            .message = "no registered toolchain named '" + name + "'",
            .location = std::nullopt,
            .notes = {}
        });
    }
}
