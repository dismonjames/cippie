#pragma once

#include <cippie/diagnostics/SourceLocation.hpp>

#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    enum class DiagnosticSeverity
    {
        error,
        warning,
        note,
        info
    };

    struct Diagnostic
    {
        DiagnosticSeverity severity{DiagnosticSeverity::error};
        std::string message;
        SourceLocation location;
        std::vector<std::string> notes;
        std::optional<std::string> sourceLine;
    };
}
