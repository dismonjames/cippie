#include <cippie/diagnostics/Logger.hpp>

#include <iostream>

namespace cippie
{
    void Logger::info(std::string_view message) const
    {
        std::cout << message << '\n';
    }

    void Logger::warning(std::string_view message) const
    {
        std::cerr << "warning: " << message << '\n';
    }

    void Logger::error(std::string_view message) const
    {
        std::cerr << "error: " << message << '\n';
    }

    void Logger::buildStep(
        std::size_t current,
        std::size_t total,
        std::string_view action,
        std::string_view subject,
        std::string_view output
    ) const
    {
        std::cout << '[' << current << '/' << total << "] " << action << ' ' << subject;
        if (!output.empty())
            std::cout << " -> " << output;
        std::cout << '\n';
    }
}
