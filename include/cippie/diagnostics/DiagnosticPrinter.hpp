#pragma once

#include <cippie/diagnostics/Diagnostic.hpp>

#include <string>
#include <vector>

namespace cippie
{
    class DiagnosticPrinter
    {
    public:
        [[nodiscard]] static std::string format(
            const Diagnostic& diagnostic,
            bool useColor = false
        );

        [[nodiscard]] static std::string formatAll(
            const std::vector<Diagnostic>& diagnostics,
            bool useColor = false
        );
    };
}
