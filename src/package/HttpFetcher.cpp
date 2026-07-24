#include <cippie/package/HttpFetcher.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/util/SHA256.hpp>

#include <fstream>
#include <system_error>

namespace cippie
{
    Result<void> HttpFetcher::downloadFile(
        const std::string& url,
        const std::filesystem::path& destinationFile,
        const std::string& expectedSha256
    )
    {
        std::error_code ec;
        std::filesystem::create_directories(destinationFile.parent_path(), ec);

        const auto tmpPath = destinationFile.string() + ".tmp-download";
        std::filesystem::remove(tmpPath, ec);

        Process process;

        if (url.starts_with("file://"))
        {
            std::filesystem::path srcFile(url.substr(7));
            std::filesystem::copy_file(srcFile, tmpPath, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to copy local file: " + srcFile.string(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }
        else
        {
            ProcessRequest req;
            req.executable = "curl";
            req.captureOutput = true;
            req.arguments = {"--fail", "--silent", "--show-error", "--location", "-o", tmpPath, url};

            auto res = process.run(req);
            if (res.exitCode != 0)
            {
                std::filesystem::remove(tmpPath, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::processFailed,
                    .message = "curl download failed for URL '" + url + "': " + res.stderrOutput,
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }

        if (!expectedSha256.empty())
        {
            auto actualShaRes = SHA256::hashFile(tmpPath);
            if (!actualShaRes.has_value())
            {
                std::filesystem::remove(tmpPath, ec);
                return std::unexpected(actualShaRes.error());
            }

            std::string expClean = expectedSha256;
            if (expClean.starts_with("sha256:")) expClean = expClean.substr(7);

            if (*actualShaRes != expClean)
            {
                std::filesystem::remove(tmpPath, ec);
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "integrity verification failed for package archive downloaded from " + url + "\n  expected: sha256:" + expClean + "\n  actual  : sha256:" + *actualShaRes,
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }

        std::filesystem::rename(tmpPath, destinationFile, ec);
        if (ec)
        {
            std::filesystem::copy_file(tmpPath, destinationFile, std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(tmpPath, ec);
        }

        return {};
    }

    Result<std::string> HttpFetcher::fetchString(const std::string& url)
    {
        if (url.starts_with("file://"))
        {
            std::filesystem::path srcFile(url.substr(7));
            std::ifstream file(srcFile);
            if (!file.is_open())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::fileReadFailed,
                    .message = "failed to open local registry file: " + srcFile.string(),
                    .location = std::nullopt,
                    .notes = {}
                });
            }
            return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }

        ProcessRequest req;
        req.executable = "curl";
        req.captureOutput = true;
        req.arguments = {"--fail", "--silent", "--show-error", "--location", url};

        Process process;
        auto res = process.run(req);

        if (res.exitCode != 0)
        {
            return std::unexpected(Error{
                .code = ErrorCode::processFailed,
                .message = "failed to fetch HTTP resource: " + res.stderrOutput,
                .location = std::nullopt,
                .notes = {}
            });
        }

        return res.stdoutOutput;
    }
}
