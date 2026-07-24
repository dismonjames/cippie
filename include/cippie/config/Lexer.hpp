#pragma once

#include <cippie/config/Token.hpp>
#include <cippie/diagnostics/Diagnostic.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace cippie
{
    class Lexer
    {
    public:
        Lexer(std::string source, std::filesystem::path filePath);

        [[nodiscard]] std::vector<Token> tokenize();
        [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;
        [[nodiscard]] bool hasErrors() const noexcept;

        [[nodiscard]] const std::string& source() const noexcept { return m_source; }
        [[nodiscard]] static std::string unescapeString(
            std::string_view rawWithQuotes,
            bool* hasError = nullptr
        );

    private:
        char peek() const noexcept;
        char peekNext() const noexcept;
        char advance() noexcept;
        bool isAtEnd() const noexcept;

        void scanToken();
        void skipWhitespaceAndComments();
        void scanIdentifierOrKeyword();
        void scanNumber();
        void scanString();

        void addToken(TokenKind kind);
        void addDiagnostic(
            DiagnosticSeverity severity,
            std::string message,
            SourceLocation location
        );

        std::string m_source;
        std::filesystem::path m_filePath;
        std::size_t m_start{0};
        std::size_t m_current{0};
        std::size_t m_line{1};
        std::size_t m_column{1};
        std::size_t m_startColumn{1};
        std::vector<Token> m_tokens;
        std::vector<Diagnostic> m_diagnostics;
        std::vector<std::string_view> m_sourceLines;
    };
}
