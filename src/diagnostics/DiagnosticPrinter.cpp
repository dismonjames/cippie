#include <cippie/diagnostics/DiagnosticPrinter.hpp>

#include <sstream>

namespace cippie
{
    namespace
    {
        std::string_view severityString(DiagnosticSeverity severity) noexcept
        {
            switch (severity)
            {
                case DiagnosticSeverity::error:
                    return "error";
                case DiagnosticSeverity::warning:
                    return "warning";
                case DiagnosticSeverity::note:
                    return "note";
                case DiagnosticSeverity::info:
                    return "info";
            }
            return "error";
        }
    }

    std::string DiagnosticPrinter::format(
        const Diagnostic& diagnostic,
        bool useColor
    )
    {
        std::ostringstream ss;
        const auto filePath = diagnostic.location.file.empty()
                                  ? "Cippiefile"
                                  : diagnostic.location.file.string();

        if (useColor)
        {
            const char* colorCode = "\033[1;31m"; // Red for error
            if (diagnostic.severity == DiagnosticSeverity::warning)
            {
                colorCode = "\033[1;33m"; // Yellow
            }
            else if (diagnostic.severity == DiagnosticSeverity::note)
            {
                colorCode = "\033[1;36m"; // Cyan
            }

            ss << filePath << ":" << diagnostic.location.line << ":"
               << diagnostic.location.column << ": " << colorCode
               << severityString(diagnostic.severity) << "\033[0m: "
               << diagnostic.message;
        }
        else
        {
            ss << filePath << ":" << diagnostic.location.line << ":"
               << diagnostic.location.column << ": "
               << severityString(diagnostic.severity) << ": "
               << diagnostic.message;
        }

        if (diagnostic.sourceLine.has_value() && !diagnostic.sourceLine->empty())
        {
            ss << "\n    " << *diagnostic.sourceLine << "\n    ";
            for (std::size_t i = 1; i < diagnostic.location.column; ++i)
            {
                ss << ' ';
            }
            ss << '^';
        }

        for (const auto& note : diagnostic.notes)
        {
            ss << "\n  note: " << note;
        }

        return ss.str();
    }

    std::string DiagnosticPrinter::formatAll(
        const std::vector<Diagnostic>& diagnostics,
        bool useColor
    )
    {
        std::ostringstream ss;
        for (std::size_t i = 0; i < diagnostics.size(); ++i)
        {
            if (i > 0)
            {
                ss << '\n';
            }
            ss << format(diagnostics[i], useColor);
        }
        return ss.str();
    }
}
