#pragma once

#include <cippie/config/Ast.hpp>
#include <cippie/config/Token.hpp>
#include <cippie/diagnostics/Diagnostic.hpp>

#include <filesystem>
#include <optional>
#include <vector>

namespace cippie
{
    class Parser
    {
    public:
        explicit Parser(
            std::vector<Token> tokens,
            std::filesystem::path filePath = {}
        );

        [[nodiscard]] std::optional<AstProjectDeclaration> parseProject();
        [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;
        [[nodiscard]] bool hasErrors() const noexcept;

    private:
        const Token& peek() const noexcept;
        const Token& previous() const noexcept;
        bool isAtEnd() const noexcept;
        bool check(TokenKind kind) const noexcept;
        bool match(TokenKind kind);
        const Token& advance();
        const Token& consume(TokenKind kind, const std::string& errorMessage);

        std::optional<AstObject> parseObject();
        std::optional<AstStatement> parseStatement();
        std::optional<AstValue> parseValue();
        std::optional<AstArray> parseArray();
        std::optional<AstCall> parseCall(const Token& identifierToken);

        void synchronize();
        void addDiagnostic(
            DiagnosticSeverity severity,
            std::string message,
            SourceLocation location
        );

        std::vector<Token> m_tokens;
        std::filesystem::path m_filePath;
        std::size_t m_current{0};
        std::vector<Diagnostic> m_diagnostics;
    };
}
