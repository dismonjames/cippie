#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    class GitFetcher
    {
    public:
        GitFetcher() = default;

        [[nodiscard]] static Result<std::string> resolveRevision(
            const std::string& url,
            const std::string& selector // tag, rev, or branch
        );

        [[nodiscard]] static Result<void> fetchAndCheckout(
            const std::string& url,
            const std::string& commit,
            const std::filesystem::path& destinationDirectory
        );
    };
}
