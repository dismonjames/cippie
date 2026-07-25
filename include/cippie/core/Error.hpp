#pragma once

#include <cippie/core/ErrorCode.hpp>
#include <cippie/diagnostics/SourceLocation.hpp>

#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    struct Error
    {
        ErrorCode code{ErrorCode::validationFailed};
        std::string message;
        std::optional<SourceLocation> location;
        std::vector<std::string> notes;
    };
}
