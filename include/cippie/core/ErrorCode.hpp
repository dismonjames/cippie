#pragma once

namespace cippie
{
    enum class ErrorCode
    {
        invalidCommandLine,
        projectNotFound,
        fileReadFailed,
        invalidToken,
        parseFailed,
        validationFailed,
        targetNotFound,
        dependencyCycle,
        compilerNotFound,
        compilerFailed,
        linkerFailed,
        processFailed,
        cacheCorrupted,
        packageResolutionFailed,
        updateFailed
    };
}
