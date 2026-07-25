#include <cippie/util/UpdateChecker.hpp>

#include <cippie/core/Version.hpp>
#include <cippie/package/HttpFetcher.hpp>
#include <cippie/process/Process.hpp>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <system_error>
#include <tuple>

#if defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <cstdint>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace cippie
{
    std::filesystem::path UpdateChecker::getConfigDir()
    {
        const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
        if (xdgConfig != nullptr && xdgConfig[0] != '\0')
            return std::filesystem::path(xdgConfig) / "cippie";

        const char* home = std::getenv("HOME");
        if (home != nullptr && home[0] != '\0')
            return std::filesystem::path(home) / ".config" / "cippie";

        return std::filesystem::temp_directory_path() / "cippie";
    }

    std::filesystem::path UpdateChecker::getLastCheckFile()
    {
        return getConfigDir() / "update-check";
    }

    std::filesystem::path UpdateChecker::getCurrentExecutablePath()
    {
#if defined(__linux__)
        std::error_code ec;
        auto path = std::filesystem::read_symlink("/proc/self/exe", ec);
        if (ec) return {};
        return path;
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buf(size, '\0');
        if (_NSGetExecutablePath(buf.data(), &size) != 0)
            return {};
        std::error_code ec;
        auto path = std::filesystem::absolute(std::filesystem::path(buf.data()), ec);
        if (ec) return {};
        return path;
#elif defined(_WIN32)
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (len == 0 || len == MAX_PATH) return {};
        return std::filesystem::absolute(std::filesystem::path(buf));
#else
        return {};
#endif
    }

    std::string UpdateChecker::getPlatformSuffix()
    {
        std::string os;
#if defined(__linux__)
        os = "linux";
#elif defined(__APPLE__)
        os = "darwin";
#elif defined(_WIN32)
        os = "windows";
#else
        os = "unknown";
#endif

        std::string arch;
#if defined(__x86_64__) || defined(_M_X64)
        arch = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
        arch = "aarch64";
#elif defined(__i386__) || defined(_M_IX86)
        arch = "i386";
#else
        arch = "unknown";
#endif

        return os + "-" + arch;
    }

    Result<bool> UpdateChecker::isCheckDue()
    {
        auto path = getLastCheckFile();
        std::error_code ec;
        if (!std::filesystem::exists(path, ec))
            return true;

        std::ifstream file(path);
        if (!file.is_open())
            return true;

        std::string line;
        std::getline(file, line);

        if (line.empty())
            return true;

        char* end = nullptr;
        auto lastEpoch = std::strtoll(line.c_str(), &end, 10);
        if (end == line.c_str())
            return true;

        auto now = std::chrono::system_clock::now();
        auto lastTime = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(lastEpoch));
        auto diff = now - lastTime;
        auto days = std::chrono::duration_cast<std::chrono::hours>(diff).count() / 24;

        return days >= 18;
    }

    void UpdateChecker::saveCheckTimestamp()
    {
        auto dir = getConfigDir();
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);

        auto path = getLastCheckFile();
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::system_clock::to_time_t(now);

        std::ofstream file(path);
        if (file.is_open())
            file << epoch << "\n";
    }

    Result<std::string> UpdateChecker::fetchLatestTag()
    {
        std::string url = "https://api.github.com/repos/dismonjames/cippie/releases/latest";
        auto res = HttpFetcher::fetchString(url);
        if (!res.has_value())
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to fetch latest release info: " + res.error().message,
                .location = {}, .notes = {}
            });

        const auto& json = *res;
        auto tagKey = "\"tag_name\"";
        auto tagPos = json.find(tagKey);
        if (tagPos == std::string::npos)
            return std::unexpected(Error{
                .code = ErrorCode::parseFailed,
                .message = "could not find tag_name in GitHub API response",
                .location = {}, .notes = {}
            });

        auto colonPos = json.find(':', tagPos + std::strlen(tagKey));
        if (colonPos == std::string::npos)
            return std::unexpected(Error{ .code = ErrorCode::parseFailed, .message = "invalid JSON", .location = {}, .notes = {} });

        auto quotePos = json.find('"', colonPos + 1);
        if (quotePos == std::string::npos)
            return std::unexpected(Error{ .code = ErrorCode::parseFailed, .message = "invalid JSON", .location = {}, .notes = {} });

        auto endQuotePos = json.find('"', quotePos + 1);
        if (endQuotePos == std::string::npos)
            return std::unexpected(Error{ .code = ErrorCode::parseFailed, .message = "invalid JSON", .location = {}, .notes = {} });

        return json.substr(quotePos + 1, endQuotePos - quotePos - 1);
    }

    Result<std::string> UpdateChecker::findAssetUrl(const std::string& json)
    {
        auto suffix = getPlatformSuffix();
        auto assetsKey = "\"assets\"";
        auto assetsPos = json.find(assetsKey);
        if (assetsPos == std::string::npos)
            return std::unexpected(Error{
                .code = ErrorCode::parseFailed,
                .message = "could not find assets in GitHub API response"
            });

        auto nameKey = std::string("\"name\"");
        auto urlKey = std::string("\"browser_download_url\"");
        auto searchPos = assetsPos;
        auto dotTarGz = std::string(".tar.gz");

        while (true)
        {
            auto namePos = json.find(nameKey, searchPos);
            if (namePos == std::string::npos) break;

            auto colonPos = json.find(':', namePos + nameKey.size());
            if (colonPos == std::string::npos) break;

            auto qPos = json.find('"', colonPos + 1);
            if (qPos == std::string::npos) break;
            auto qEnd = json.find('"', qPos + 1);
            if (qEnd == std::string::npos) break;

            auto assetName = json.substr(qPos + 1, qEnd - qPos - 1);
            searchPos = qEnd + 1;

            if (!assetName.ends_with(dotTarGz))
                continue;

            auto expectedSuffix = std::string("-") + suffix + dotTarGz;
            if (assetName.size() < expectedSuffix.size())
                continue;

            if (!assetName.ends_with(expectedSuffix))
                continue;

            // Find the download URL in the same asset object
            auto objStart = json.rfind('{', namePos);
            auto objEnd = json.find('}', namePos);
            if (objStart == std::string::npos || objEnd == std::string::npos)
                continue;

            auto assetJson = json.substr(objStart, objEnd - objStart + 1);
            auto urlPos = assetJson.find(urlKey);
            if (urlPos == std::string::npos) continue;

            auto urlColon = assetJson.find(':', urlPos + urlKey.size());
            if (urlColon == std::string::npos) continue;

            auto urlQ = assetJson.find('"', urlColon + 1);
            if (urlQ == std::string::npos) continue;
            auto urlQEnd = assetJson.find('"', urlQ + 1);
            if (urlQEnd == std::string::npos) continue;

            return assetJson.substr(urlQ + 1, urlQEnd - urlQ - 1);
        }

        return std::unexpected(Error{
            .code = ErrorCode::validationFailed,
            .message = "no release asset found for platform " + suffix
        });
    }

    int UpdateChecker::compareVersions(const std::string& v1, const std::string& v2)
    {
        auto parse = [](const std::string& v) -> std::tuple<int, int, int>
        {
            std::string s = v;
            if (s.starts_with('v')) s = s.substr(1);
            int maj = 0, min = 0, pat = 0;
            std::istringstream ss(s);
            char dot;
            ss >> maj >> dot >> min >> dot >> pat;
            return {maj, min, pat};
        };

        auto [m1, n1, p1] = parse(v1);
        auto [m2, n2, p2] = parse(v2);

        if (m1 != m2) return m1 < m2 ? -1 : 1;
        if (n1 != n2) return n1 < n2 ? -1 : 1;
        if (p1 != p2) return p1 < p2 ? -1 : 1;
        return 0;
    }

    Result<void> UpdateChecker::downloadAndReplace(const std::string& tag, const std::string& assetUrl)
    {
        auto versionStr = tag;
        if (versionStr.starts_with('v'))
            versionStr = versionStr.substr(1);

        auto suffix = getPlatformSuffix();
        auto tarballName = "cippie-" + versionStr + "-" + suffix + ".tar.gz";

        std::random_device rd;
        auto tmpDir = std::filesystem::temp_directory_path() / ("cippie-update-" + std::to_string(rd()));
        std::error_code ec;
        std::filesystem::create_directories(tmpDir, ec);
        if (ec)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to create temp directory"
            });
        }

        auto tarballPath = tmpDir / tarballName;

        // Download checksum
        auto checksumUrl = assetUrl + ".sha256";
        auto checksumRes = HttpFetcher::fetchString(checksumUrl);
        if (!checksumRes.has_value())
        {
            std::filesystem::remove_all(tmpDir, ec);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to download checksum: " + checksumRes.error().message
            });
        }

        std::string expectedHash;
        {
            auto& cs = *checksumRes;
            auto spacePos = cs.find(' ');
            if (spacePos != std::string::npos)
                expectedHash = cs.substr(0, spacePos);
            else
                expectedHash = cs;
        }

        // Download tarball with verification
        auto dlRes = HttpFetcher::downloadFile(assetUrl, tarballPath, expectedHash);
        if (!dlRes.has_value())
        {
            std::filesystem::remove_all(tmpDir, ec);
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to download release: " + dlRes.error().message
            });
        }

        // Extract tarball
        auto extractDir = tmpDir / "extract";
        std::filesystem::create_directories(extractDir, ec);

        {
            ProcessRequest req;
            req.executable = "tar";
            req.arguments = {"-xzf", tarballPath.string(), "-C", extractDir.string()};

            Process proc;
            auto pres = proc.run(req);
            if (pres.exitCode != 0)
            {
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::processFailed,
                    .message = "failed to extract release tarball"
                });
            }
        }

        // Find the extracted binary
        auto packageDir = extractDir / ("cippie-" + versionStr + "-" + suffix);
        auto extractedBinary = packageDir / "bin" / "cippie";
#if defined(_WIN32)
        extractedBinary.replace_extension(".exe");
#endif

        if (!std::filesystem::exists(extractedBinary, ec))
        {
            // Fallback: search for cippie binary anywhere in extraction
            bool found = false;
            for (auto& entry : std::filesystem::recursive_directory_iterator(extractDir, ec))
            {
                if (entry.is_regular_file() &&
                    entry.path().filename() ==
#if defined(_WIN32)
                    "cippie.exe"
#else
                    "cippie"
#endif
                )
                {
                    extractedBinary = entry.path();
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "extracted archive does not contain cippie binary"
                });
            }
        }

        // Verify extracted binary
        {
            ProcessRequest verReq;
            verReq.executable = extractedBinary.string();
            verReq.arguments = {"version"};

            Process verProc;
            auto vres = verProc.run(verReq);
            if (vres.exitCode != 0)
            {
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::processFailed,
                    .message = "extracted binary failed to run"
                });
            }
        }

        // Replace current binary
        auto currentPath = getCurrentExecutablePath();
        if (currentPath.empty())
        {
            std::filesystem::remove_all(tmpDir, ec);
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "could not determine current executable path"
            });
        }

#if defined(_WIN32)
        {
            auto oldPath = currentPath;
            oldPath.replace_extension(".exe.old");
            std::filesystem::rename(currentPath, oldPath, ec);
            if (ec)
            {
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to rename current executable"
                });
            }

            std::filesystem::copy_file(extractedBinary, currentPath, ec);
            if (ec)
            {
                std::filesystem::rename(oldPath, currentPath, ec);
                std::filesystem::remove_all(tmpDir, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to copy new executable"
                });
            }
            std::error_code ignore;
            std::filesystem::remove(oldPath, ignore);
        }
#else
        {
            std::filesystem::rename(extractedBinary, currentPath, ec);
            if (ec)
            {
                std::filesystem::copy_file(extractedBinary, currentPath,
                    std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                {
                    std::filesystem::remove_all(tmpDir, ec);
                    return std::unexpected(Error{
                        .code = ErrorCode::fileReadFailed,
                        .message = "failed to replace executable: " + ec.message()
                    });
                }
            }

            auto perms = std::filesystem::perms::owner_exec |
                         std::filesystem::perms::group_exec |
                         std::filesystem::perms::others_exec;
            std::filesystem::permissions(currentPath, perms,
                std::filesystem::perm_options::add, ec);
        }
#endif

        std::filesystem::remove_all(tmpDir, ec);
        return {};
    }

    Result<void> UpdateChecker::performUpdate(bool force)
    {
        auto selfPath = getCurrentExecutablePath();
        if (selfPath.empty())
        {
            std::cout << "warning: could not determine current executable path, skipping self-update\n";
            return {};
        }

        auto dueRes = isCheckDue();
        if (!dueRes.has_value())
            return std::unexpected(dueRes.error());

        if (!*dueRes && !force)
        {
            auto lastCheckPath = getLastCheckFile();
            std::ifstream file(lastCheckPath);
            std::string lastCheckStr;
            if (file.is_open())
                std::getline(file, lastCheckStr);

            if (!lastCheckStr.empty())
            {
                char* end = nullptr;
                auto lastEpoch = std::strtoll(lastCheckStr.c_str(), &end, 10);
                if (end != lastCheckStr.c_str())
                {
                    auto lastTime = std::chrono::system_clock::from_time_t(
                        static_cast<std::time_t>(lastEpoch));
                    auto timeT = std::chrono::system_clock::to_time_t(lastTime);
                    std::tm* tmPtr = std::gmtime(&timeT);
                    if (tmPtr != nullptr)
                    {
                        char buf[64];
                        std::strftime(buf, sizeof(buf), "%Y-%m-%d", tmPtr);
                        auto nextCheck = lastTime + std::chrono::hours(18 * 24);
                        auto nextT = std::chrono::system_clock::to_time_t(nextCheck);
                        std::tm* nextTm = std::gmtime(&nextT);
                        char nextBuf[64];
                        std::strftime(nextBuf, sizeof(nextBuf), "%Y-%m-%d", nextTm);

                        std::cout << "Last checked for updates: " << buf
                                  << ". Next check after: " << nextBuf
                                  << ". Use --force to check now.\n";
                        return {};
                    }
                }
            }

            std::cout << "Use --force to check for updates now.\n";
            return {};
        }

        auto currentVer = std::string(version());

        std::cout << "Checking for updates...\n";

        auto tagRes = fetchLatestTag();
        if (!tagRes.has_value())
        {
            std::cerr << "error: " << tagRes.error().message << "\n";
            return {};
        }

        auto& latestTag = *tagRes;
        auto cmp = compareVersions(latestTag, "v" + currentVer);

        if (cmp <= 0)
        {
            saveCheckTimestamp();
            std::cout << "Already up to date (v" << currentVer << ")\n";
            return {};
        }

        std::cout << "New version available: " << latestTag << " (current: v" << currentVer << ")\n";

        // Fetch release data to get asset URLs
        std::string url = "https://api.github.com/repos/dismonjames/cippie/releases/latest";
        auto jsonRes = HttpFetcher::fetchString(url);
        if (!jsonRes.has_value())
        {
            std::cerr << "error: failed to fetch release data: " << jsonRes.error().message << "\n";
            return {};
        }

        auto assetUrlRes = findAssetUrl(*jsonRes);
        if (!assetUrlRes.has_value())
        {
            std::cerr << "error: " << assetUrlRes.error().message << "\n";
            return {};
        }

        std::cout << "Downloading " << latestTag << "...\n";

        auto dlRes = downloadAndReplace(latestTag, *assetUrlRes);
        if (!dlRes.has_value())
        {
            std::cerr << "error: update failed: " << dlRes.error().message << "\n";
            return {};
        }

        saveCheckTimestamp();
        std::cout << "Updated to " << latestTag << "!\n";
        std::cout << "Please restart Cippie to use the new version.\n";

        return {};
    }
}
