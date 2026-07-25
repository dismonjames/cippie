#include <cippie/build/SourceScanner.hpp>

#include <algorithm>
#include <system_error>

namespace cippie
{
    namespace
    {
        bool isIgnoredDir(const std::filesystem::path& path)
        {
            const auto filename = path.filename().string();
            if (filename == ".git" || filename == ".cippie" || filename == "build")
            {
                return true;
            }
            if (filename.rfind("build-", 0) == 0 || filename.rfind("build_", 0) == 0)
            {
                return true;
            }
            if (filename.rfind("cmake-build-", 0) == 0)
            {
                return true;
            }
            if (!filename.empty() && filename.front() == '.' && filename != "." && filename != "..")
            {
                return true;
            }
            return false;
        }

        bool isCppSourceExtension(const std::filesystem::path& path)
        {
            const auto ext = path.extension().string();
            return ext == ".cpp" || ext == ".cc" || ext == ".cxx";
        }

        bool matchesPatternExtension(const std::filesystem::path& path, const std::string& pattern)
        {
            if (!isCppSourceExtension(path))
            {
                return false;
            }
            auto ext = path.extension().string();
            if (pattern.ends_with(".cpp") && ext != ".cpp") return false;
            if (pattern.ends_with(".cc") && ext != ".cc") return false;
            if (pattern.ends_with(".cxx") && ext != ".cxx") return false;
            return true;
        }
    }

    std::vector<std::filesystem::path> SourceScanner::scan(
        const std::filesystem::path& rootDir,
        const std::vector<std::string>& patterns,
        const std::optional<std::filesystem::path>& entry
    ) const
    {
        std::vector<std::filesystem::path> sources;

        if (entry.has_value() && std::filesystem::is_regular_file(*entry))
        {
            sources.push_back(entry->lexically_normal());
        }

        std::error_code ec;
        if (!std::filesystem::is_directory(rootDir, ec))
        {
            if (!sources.empty())
            {
                std::sort(sources.begin(), sources.end());
                sources.erase(std::unique(sources.begin(), sources.end()), sources.end());
            }
            return sources;
        }

        if (patterns.empty())
        {
            if (entry.has_value())
            {
                // If entry is explicitly set and patterns is empty, sources is just entry.
                return sources;
            }

            // Default recursive scan under rootDir when no patterns and no entry specified
            std::filesystem::recursive_directory_iterator iter(
                rootDir,
                std::filesystem::directory_options::skip_permission_denied,
                ec
            );
            const std::filesystem::recursive_directory_iterator end;

            while (iter != end)
            {
                if (ec)
                {
                    ec.clear();
                    iter.increment(ec);
                    continue;
                }

                if (iter->is_directory(ec) && isIgnoredDir(iter->path()))
                {
                    iter.disable_recursion_pending();
                }
                else if (iter->is_regular_file(ec) && isCppSourceExtension(iter->path()))
                {
                    sources.push_back(iter->path().lexically_normal());
                }

                iter.increment(ec);
            }
        }
        else
        {
            for (const auto& pattern : patterns)
            {
                bool isRecursive = (pattern.find("**") != std::string::npos);

                // Determine base directory prefix before any wildcard
                std::string baseSubdir;
                auto starPos = pattern.find('*');
                if (starPos != std::string::npos)
                {
                    auto lastSlash = pattern.rfind('/', starPos);
                    if (lastSlash != std::string::npos)
                    {
                        baseSubdir = pattern.substr(0, lastSlash);
                    }
                }
                else
                {
                    baseSubdir = pattern;
                }

                std::filesystem::path baseDir = rootDir;
                if (!baseSubdir.empty())
                {
                    baseDir /= baseSubdir;
                }

                if (!std::filesystem::is_directory(baseDir, ec))
                {
                    // Check if pattern directly names a file
                    auto directFile = rootDir / pattern;
                    if (std::filesystem::is_regular_file(directFile, ec) &&
                        matchesPatternExtension(directFile, pattern))
                    {
                        sources.push_back(directFile.lexically_normal());
                    }
                    continue;
                }

                if (isRecursive)
                {
                    std::filesystem::recursive_directory_iterator iter(
                        baseDir,
                        std::filesystem::directory_options::skip_permission_denied,
                        ec
                    );
                    const std::filesystem::recursive_directory_iterator end;

                    while (iter != end)
                    {
                        if (ec)
                        {
                            ec.clear();
                            iter.increment(ec);
                            continue;
                        }

                        if (iter->is_directory(ec) && isIgnoredDir(iter->path()))
                        {
                            iter.disable_recursion_pending();
                        }
                        else if (iter->is_regular_file(ec) && matchesPatternExtension(iter->path(), pattern))
                        {
                            sources.push_back(iter->path().lexically_normal());
                        }

                        iter.increment(ec);
                    }
                }
                else
                {
                    // Single-level directory scan
                    std::filesystem::directory_iterator iter(baseDir, ec);
                    const std::filesystem::directory_iterator end;

                    while (iter != end)
                    {
                        if (ec)
                        {
                            ec.clear();
                            iter.increment(ec);
                            continue;
                        }

                        if (iter->is_regular_file(ec) && matchesPatternExtension(iter->path(), pattern))
                        {
                            sources.push_back(iter->path().lexically_normal());
                        }

                        iter.increment(ec);
                    }
                }
            }
        }

        std::sort(sources.begin(), sources.end());
        sources.erase(std::unique(sources.begin(), sources.end()), sources.end());

        return sources;
    }

    std::vector<std::filesystem::path> SourceScanner::scan(
        const std::filesystem::path& sourceDirectory
    ) const
    {
        return scan(sourceDirectory, {});
    }
}
