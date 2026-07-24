#include <cippie/package/GitFetcher.hpp>
#include <cippie/process/Process.hpp>

#include <algorithm>
#include <sstream>
#include <system_error>

namespace cippie
{
    Result<std::string> GitFetcher::resolveRevision(
        const std::string& url,
        const std::string& selector
    )
    {
        // If selector is already a full 40-character commit hash
        if (selector.size() == 40 && std::all_of(selector.begin(), selector.end(), [](char c) {
                return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            }))
        {
            return selector;
        }

        ProcessRequest req;
        req.executable = "git";
        req.captureOutput = true;

        if (selector.empty())
        {
            req.arguments = {"ls-remote", url, "HEAD"};
        }
        else
        {
            req.arguments = {"ls-remote", url, selector, "refs/tags/" + selector, "refs/heads/" + selector};
        }

        Process process;
        auto res = process.run(req);

        if (res.exitCode != 0 || res.stdoutOutput.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "git ls-remote failed for url '" + url + "' selector '" + selector + "': " + res.stderrOutput,
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::stringstream ss(res.stdoutOutput);
        std::string commit;
        ss >> commit;

        if (commit.size() < 40)
        {
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to parse commit hash from git output: " + res.stdoutOutput,
                .location = std::nullopt,
                .notes = {}
            });
        }

        return commit.substr(0, 40);
    }

    Result<void> GitFetcher::fetchAndCheckout(
        const std::string& url,
        const std::string& commit,
        const std::filesystem::path& destinationDirectory
    )
    {
        std::error_code ec;
        if (std::filesystem::is_regular_file(destinationDirectory / "Cippiefile", ec))
        {
            return {}; // Already checked out and valid
        }

        std::filesystem::create_directories(destinationDirectory, ec);

        Process process;

        ProcessRequest initReq{.executable = "git", .arguments = {"init"}, .workingDirectory = destinationDirectory, .captureOutput = true};
        if (process.run(initReq).exitCode != 0)
        {
            return std::unexpected(Error{.code = ErrorCode::processFailed, .message = "git init failed", .location = std::nullopt, .notes = {}});
        }

        ProcessRequest remoteReq{.executable = "git", .arguments = {"remote", "add", "origin", url}, .workingDirectory = destinationDirectory, .captureOutput = true};
        (void)process.run(remoteReq);

        ProcessRequest fetchReq{.executable = "git", .arguments = {"fetch", "--depth", "1", "origin", commit}, .workingDirectory = destinationDirectory, .captureOutput = true};
        auto fetchRes = process.run(fetchReq);

        if (fetchRes.exitCode != 0)
        {
            ProcessRequest fallbackFetch{.executable = "git", .arguments = {"fetch", "origin"}, .workingDirectory = destinationDirectory, .captureOutput = true};
            if (process.run(fallbackFetch).exitCode != 0)
            {
                return std::unexpected(Error{.code = ErrorCode::processFailed, .message = "git fetch failed for " + url, .location = std::nullopt, .notes = {}});
            }
        }

        ProcessRequest checkoutReq{.executable = "git", .arguments = {"checkout", "-q", "-f", commit}, .workingDirectory = destinationDirectory, .captureOutput = true};
        if (process.run(checkoutReq).exitCode != 0)
        {
            return std::unexpected(Error{.code = ErrorCode::processFailed, .message = "git checkout failed for commit " + commit, .location = std::nullopt, .notes = {}});
        }

        return {};
    }
}
