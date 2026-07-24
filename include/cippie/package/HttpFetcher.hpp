#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    class HttpFetcher
    {
    public:
        HttpFetcher() = default;

        [[nodiscard]] static Result<void> downloadFile(
            const std::string& url,
            const std::filesystem::path& destinationFile,
            const std::string& expectedSha256 = ""
        );

        [[nodiscard]] static Result<std::string> fetchString(
            const std::string& url
        );
    };
}
