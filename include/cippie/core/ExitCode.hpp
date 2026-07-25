#pragma once

namespace cippie
{
    enum class ExitCode : int
    {
        success = 0,
        generalError = 1,
        invalidArguments = 2,
        projectNotFound = 3,
        configurationError = 4,
        dependencyResolutionFailed = 5,
        buildFailed = 6,
        testFailed = 7,
        toolchainFailed = 8,
        packageFailed = 9,
        internalFailure = 10,
        updateFailed = 11
    };

    [[nodiscard]] constexpr int toInt(ExitCode code) noexcept
    {
        return static_cast<int>(code);
    }
}
