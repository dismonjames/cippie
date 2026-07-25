#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    class UpdateChecker
    {
    public:
        static Result<void> performUpdate(bool force);

    private:
        static std::filesystem::path getConfigDir();
        static std::filesystem::path getLastCheckFile();
        static std::filesystem::path getCurrentExecutablePath();
        static std::string getPlatformSuffix();
        static Result<bool> isCheckDue();
        static void saveCheckTimestamp();
        static Result<std::string> fetchLatestTag();
        static Result<std::string> findAssetUrl(const std::string& json);
        static int compareVersions(const std::string& v1, const std::string& v2);
        static Result<void> downloadAndReplace(const std::string& tag, const std::string& assetUrl);
    };
}
