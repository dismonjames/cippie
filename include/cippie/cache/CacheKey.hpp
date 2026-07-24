#pragma once

#include <cippie/cache/FileHasher.hpp>

#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace cippie
{
    class CacheKeyBuilder
    {
    public:
        CacheKeyBuilder() = default;

        static constexpr uint32_t SCHEMA_VERSION = 1;

        static std::string buildCompileKey(
            std::string_view compilerPath,
            std::string_view compilerVersion,
            std::string_view compilerFamily,
            std::string_view targetTriple,
            std::string_view configuration,
            int cppStandard,
            bool picEnabled,
            const std::filesystem::path& sourcePath,
            std::string_view sourceHash,
            const std::vector<std::filesystem::path>& includeDirs,
            const std::vector<std::string>& definitions,
            const std::vector<std::string>& options,
            const std::vector<std::pair<std::filesystem::path, std::string>>& headerDeps
        )
        {
            std::ostringstream ss;
            ss << "v:" << SCHEMA_VERSION << "\n";
            ss << "cp:" << compilerPath << "\n";
            ss << "cv:" << compilerVersion << "\n";
            ss << "cf:" << compilerFamily << "\n";
            ss << "tt:" << targetTriple << "\n";
            ss << "cfg:" << configuration << "\n";
            ss << "std:" << cppStandard << "\n";
            ss << "pic:" << (picEnabled ? "1" : "0") << "\n";
            ss << "src:" << sourcePath.lexically_normal().string() << ":" << sourceHash << "\n";

            ss << "inc:" << includeDirs.size() << "\n";
            for (const auto& inc : includeDirs)
            {
                ss << "  " << inc.lexically_normal().string() << "\n";
            }

            ss << "def:" << definitions.size() << "\n";
            for (const auto& def : definitions)
            {
                ss << "  " << def << "\n";
            }

            ss << "opt:" << options.size() << "\n";
            for (const auto& opt : options)
            {
                ss << "  " << opt << "\n";
            }

            ss << "dep:" << headerDeps.size() << "\n";
            for (const auto& dep : headerDeps)
            {
                ss << "  " << dep.first.lexically_normal().string() << ":" << dep.second << "\n";
            }

            return FileHasher::hashString(ss.str());
        }

        static std::string buildLinkKey(
            std::string_view linkerPath,
            std::string_view linkerVersion,
            std::string_view targetTriple,
            std::string_view configuration,
            const std::vector<std::string>& linkOptions,
            const std::vector<std::string>& objectHashes,
            const std::vector<std::string>& libraryHashes
        )
        {
            std::ostringstream ss;
            ss << "v:" << SCHEMA_VERSION << "\n";
            ss << "lp:" << linkerPath << "\n";
            ss << "lv:" << linkerVersion << "\n";
            ss << "tt:" << targetTriple << "\n";
            ss << "cfg:" << configuration << "\n";

            ss << "opt:" << linkOptions.size() << "\n";
            for (const auto& opt : linkOptions)
            {
                ss << "  " << opt << "\n";
            }

            ss << "obj:" << objectHashes.size() << "\n";
            for (const auto& h : objectHashes)
            {
                ss << "  " << h << "\n";
            }

            ss << "lib:" << libraryHashes.size() << "\n";
            for (const auto& h : libraryHashes)
            {
                ss << "  " << h << "\n";
            }

            return FileHasher::hashString(ss.str());
        }
    };
}
