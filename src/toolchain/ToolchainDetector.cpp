#include <cippie/toolchain/ToolchainDetector.hpp>

#include <cippie/toolchain/ToolchainRegistry.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace cippie
{
    namespace
    {
        std::optional<std::filesystem::path> findInPath(const std::string& candidate)
        {
            if (candidate.empty()) return std::nullopt;

            if (candidate.find('/') != std::string::npos)
            {
                if (std::filesystem::exists(candidate))
                {
                    return std::filesystem::absolute(candidate);
                }
                return std::nullopt;
            }

            const char* pathEnv = std::getenv("PATH");
            if (!pathEnv) return std::nullopt;

            std::stringstream ss(pathEnv);
            std::string dir;
            while (std::getline(ss, dir, ':'))
            {
                if (dir.empty()) continue;
                auto fullPath = std::filesystem::path(dir) / candidate;
                std::error_code ec;
                if (std::filesystem::is_regular_file(fullPath, ec))
                {
                    return std::filesystem::absolute(fullPath);
                }
            }

            return std::nullopt;
        }

        std::string getCompilerVersion(const std::filesystem::path& compilerPath)
        {
            std::string cmd = "\"" + compilerPath.string() + "\" --version 2>&1";
            FILE* pipe = popen(cmd.c_str(), "r");
            if (!pipe) return "unknown";

            std::array<char, 256> buffer;
            std::string result;
            if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
            {
                result = buffer.data();
                while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
                    result.pop_back();
            }
            pclose(pipe);
            return result.empty() ? "unknown" : result;
        }

        Result<Toolchain> detectNative()
        {
            Toolchain toolchain;
            toolchain.host = TargetTriple::detectHost();
            toolchain.target = toolchain.host;

            std::vector<std::string> candidates;

            if (const char* cxxEnv = std::getenv("CXX"))
            {
                if (*cxxEnv != '\0') candidates.push_back(cxxEnv);
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

                    if (candidate.find("clang") != std::string::npos)
                    {
                        toolchain.name = "clang";
                        toolchain.compilerFamily = CompilerFamily::clang;
                    }
                    else
                    {
                        toolchain.name = "gcc";
                        toolchain.compilerFamily = CompilerFamily::gcc;
                    }

                    // CC
                    if (const char* ccEnv = std::getenv("CC"))
                    {
                        if (*ccEnv != '\0')
                        {
                            auto cc = findInPath(ccEnv);
                            if (cc.has_value()) toolchain.cCompiler = *cc;
                        }
                    }
                    else
                    {
                        std::string ccName = (toolchain.compilerFamily == CompilerFamily::clang) ? "clang" : "gcc";
                        auto cc = findInPath(ccName);
                        if (cc.has_value()) toolchain.cCompiler = *cc;
                    }

                    // AR
                    if (const char* arEnv = std::getenv("AR"))
                    {
                        if (*arEnv != '\0')
                        {
                            auto ar = findInPath(arEnv);
                            if (ar.has_value()) toolchain.archiver = *ar;
                        }
                    }
                    else
                    {
                        auto ar = findInPath("ar");
                        if (ar.has_value()) toolchain.archiver = *ar;
                    }

                    toolchain.version = getCompilerVersion(*found);
                    return toolchain;
                }
            }

            return std::unexpected(Error{
                .code = ErrorCode::compilerNotFound,
                .message = "no C++ compiler found in PATH (tried clang++, g++, c++); set the CXX environment variable",
                .location = std::nullopt,
                .notes = {}
            });
        }
    }

    Result<Toolchain> ToolchainDetector::detect(const DetectOptions& options) const
    {
        // Priority 1: explicit toolchain name from --toolchain flag
        if (options.toolchainName.has_value() && !options.toolchainName->empty())
        {
            auto regRes = ToolchainRegistry::load();
            if (!regRes.has_value()) return std::unexpected(regRes.error());
            auto tcRes = regRes->findByName(*options.toolchainName);
            if (!tcRes.has_value()) return std::unexpected(tcRes.error());
            auto tc = *tcRes;
            tc.version = getCompilerVersion(tc.cxxCompiler);
            return tc;
        }

        // Priority 2: explicit target triple from --target flag
        if (options.targetTripleStr.has_value() && !options.targetTripleStr->empty())
        {
            auto tripleRes = TargetTriple::parse(*options.targetTripleStr);
            if (!tripleRes.has_value()) return std::unexpected(tripleRes.error());
            const auto& triple = *tripleRes;

            // If target == host, just use native detection
            auto host = TargetTriple::detectHost();
            if (triple == host)
            {
                auto nativeRes = detectNative();
                if (nativeRes.has_value()) nativeRes->target = triple;
                return nativeRes;
            }

            // Otherwise look up in registry
            auto regRes = ToolchainRegistry::load();
            if (!regRes.has_value()) return std::unexpected(regRes.error());
            auto tcRes = regRes->findForTarget(triple);
            if (!tcRes.has_value()) return std::unexpected(tcRes.error());
            auto tc = *tcRes;
            tc.version = getCompilerVersion(tc.cxxCompiler);
            return tc;
        }

        // Priority 3: check env vars and PATH
        return detectNative();
    }
}
