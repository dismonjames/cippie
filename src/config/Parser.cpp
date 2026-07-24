#include <cippie/config/Parser.hpp>
#include <cippie/config/Lexer.hpp>

#include <memory>
#include <stdexcept>

namespace cippie
{
    namespace
    {
        bool isIdentifierLike(TokenKind kind) noexcept
        {
            switch (kind)
            {
                case TokenKind::identifier:
                case TokenKind::projectKeyword:
                case TokenKind::executableKeyword:
                case TokenKind::libraryKeyword:
                case TokenKind::testKeyword:
                case TokenKind::packageKeyword:
                case TokenKind::dependencyKeyword:
                    return true;
                default:
                    return false;
            }
        }
    }

    Parser::Parser(std::vector<Token> tokens, std::filesystem::path filePath)
        : m_tokens(std::move(tokens))
        , m_filePath(std::move(filePath))
    {
    }

    const Token& Parser::peek() const noexcept
    {
        if (m_current >= m_tokens.size())
        {
            return m_tokens.back();
        }
        return m_tokens[m_current];
    }

    const Token& Parser::previous() const noexcept
    {
        if (m_current == 0)
        {
            return m_tokens.front();
        }
        return m_tokens[m_current - 1];
    }

    bool Parser::isAtEnd() const noexcept
    {
        return peek().kind == TokenKind::endOfFile;
    }

    bool Parser::check(TokenKind kind) const noexcept
    {
        if (isAtEnd())
        {
            return kind == TokenKind::endOfFile;
        }
        return peek().kind == kind;
    }

    bool Parser::match(TokenKind kind)
    {
        if (check(kind))
        {
            advance();
            return true;
        }
        return false;
    }

    const Token& Parser::advance()
    {
        if (!isAtEnd())
        {
            m_current++;
        }
        return previous();
    }

    const Token& Parser::consume(TokenKind kind, const std::string& errorMessage)
    {
        if (check(kind))
        {
            return advance();
        }

        addDiagnostic(DiagnosticSeverity::error, errorMessage, peek().location);
        return peek();
    }

    void Parser::addDiagnostic(
        DiagnosticSeverity severity,
        std::string message,
        SourceLocation location
    )
    {
        m_diagnostics.push_back(Diagnostic{
            .severity = severity,
            .message = std::move(message),
            .location = location,
            .notes = {},
            .sourceLine = std::nullopt
        });
    }

    const std::vector<Diagnostic>& Parser::diagnostics() const noexcept
    {
        return m_diagnostics;
    }

    bool Parser::hasErrors() const noexcept
    {
        for (const auto& diag : m_diagnostics)
        {
            if (diag.severity == DiagnosticSeverity::error)
            {
                return true;
            }
        }
        return false;
    }

    void Parser::synchronize()
    {
        advance();

        while (!isAtEnd())
        {
            if (previous().kind == TokenKind::semicolon ||
                previous().kind == TokenKind::rightBrace)
            {
                return;
            }

            switch (peek().kind)
            {
                case TokenKind::projectKeyword:
                case TokenKind::executableKeyword:
                case TokenKind::libraryKeyword:
                case TokenKind::testKeyword:
                    return;
                default:
                    break;
            }

            advance();
        }
    }

    std::optional<AstProjectDeclaration> Parser::parseProject()
    {
        m_current = 0;
        m_diagnostics.clear();

        if (!match(TokenKind::projectKeyword))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected 'project' declaration",
                peek().location
            );
            return std::nullopt;
        }

        const SourceLocation projLoc = previous().location;

        if (!match(TokenKind::leftParenthesis))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected '(' after 'project'",
                peek().location
            );
            synchronize();
            return std::nullopt;
        }

        if (!check(TokenKind::stringLiteral))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected string literal for project name",
                peek().location
            );
            synchronize();
            return std::nullopt;
        }

        const Token nameToken = advance();
        const std::string projName = Lexer::unescapeString(nameToken.lexeme);

        if (!match(TokenKind::rightParenthesis))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected ')' after project name",
                peek().location
            );
            synchronize();
            return std::nullopt;
        }

        auto body = parseObject();
        if (!body.has_value())
        {
            return std::nullopt;
        }

        return AstProjectDeclaration{
            .name = projName,
            .body = std::move(*body),
            .location = projLoc
        };
    }

    std::optional<AstObject> Parser::parseObject()
    {
        if (!match(TokenKind::leftBrace))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected '{' to open block",
                peek().location
            );
            return std::nullopt;
        }

        const SourceLocation objLoc = previous().location;
        AstObject obj{.statements = {}, .location = objLoc};

        while (!check(TokenKind::rightBrace) && !isAtEnd())
        {
            auto stmt = parseStatement();
            if (stmt.has_value())
            {
                obj.statements.push_back(std::move(*stmt));
            }
            else
            {
                synchronize();
            }
        }

        if (!match(TokenKind::rightBrace))
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "expected '}' to close block",
                peek().location
            );
            return std::nullopt;
        }

        return obj;
    }

    std::optional<AstStatement> Parser::parseStatement()
    {
        if (check(TokenKind::executableKeyword) ||
            check(TokenKind::libraryKeyword) ||
            check(TokenKind::testKeyword))
        {
            const Token kwToken = advance();
            const std::string targetKind = std::string(kwToken.lexeme);
            const SourceLocation loc = kwToken.location;

            if (!match(TokenKind::leftParenthesis))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "expected '(' after target type",
                    peek().location
                );
                return std::nullopt;
            }

            if (!check(TokenKind::stringLiteral))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "expected string literal for target name",
                    peek().location
                );
                return std::nullopt;
            }

            const Token nameToken = advance();
            const std::string targetName = Lexer::unescapeString(nameToken.lexeme);

            if (!match(TokenKind::rightParenthesis))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "expected ')' after target name",
                    peek().location
                );
                return std::nullopt;
            }

            auto body = parseObject();
            if (!body.has_value())
            {
                return std::nullopt;
            }

            return AstTargetDeclaration{
                .targetKind = targetKind,
                .name = targetName,
                .body = std::make_shared<AstObject>(std::move(*body)),
                .location = loc
            };
        }

        if (isIdentifierLike(peek().kind))
        {
            const Token idToken = advance();
            const std::string idName = std::string(idToken.lexeme);
            const SourceLocation loc = idToken.location;

            if (match(TokenKind::equal))
            {
                auto val = parseValue();
                if (!val.has_value())
                {
                    return std::nullopt;
                }

                if (!match(TokenKind::semicolon))
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "expected ';' after assignment",
                        peek().location
                    );
                    return std::nullopt;
                }

                return AstAssignment{
                    .key = idName,
                    .value = std::move(*val),
                    .location = loc
                };
            }

            if (check(TokenKind::leftBrace))
            {
                auto body = parseObject();
                if (!body.has_value())
                {
                    return std::nullopt;
                }

                return AstNamedObjectDeclaration{
                    .name = idName,
                    .body = std::make_shared<AstObject>(std::move(*body)),
                    .location = loc
                };
            }

            addDiagnostic(
                DiagnosticSeverity::error,
                "expected '=' or '{' after identifier",
                peek().location
            );
            return std::nullopt;
        }

        addDiagnostic(
            DiagnosticSeverity::error,
            "unexpected token in statement",
            peek().location
        );
        return std::nullopt;
    }

    std::optional<AstValue> Parser::parseValue()
    {
        if (check(TokenKind::stringLiteral))
        {
            const Token tok = advance();
            return AstString{
                .value = Lexer::unescapeString(tok.lexeme),
                .location = tok.location
            };
        }

        if (check(TokenKind::integerLiteral))
        {
            const Token tok = advance();
            std::int64_t val = 0;
            try
            {
                val = std::stoll(std::string(tok.lexeme));
            }
            catch (...)
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "invalid integer literal",
                    tok.location
                );
            }
            return AstInteger{
                .value = val,
                .location = tok.location
            };
        }

        if (check(TokenKind::trueKeyword))
        {
            const Token tok = advance();
            return AstBoolean{
                .value = true,
                .location = tok.location
            };
        }

        if (check(TokenKind::falseKeyword))
        {
            const Token tok = advance();
            return AstBoolean{
                .value = false,
                .location = tok.location
            };
        }

        if (isIdentifierLike(peek().kind))
        {
            const Token tok = advance();
            if (check(TokenKind::leftParenthesis))
            {
                return parseCall(tok);
            }
            if (check(TokenKind::leftBrace))
            {
                m_current--; // step back to leftBrace context
                auto obj = parseObject();
                if (obj.has_value())
                {
                    return std::make_shared<AstObject>(std::move(*obj));
                }
                return std::nullopt;
            }
            return AstIdentifier{
                .name = std::string(tok.lexeme),
                .location = tok.location
            };
        }

        if (check(TokenKind::leftBracket))
        {
            auto arr = parseArray();
            if (arr.has_value())
            {
                return *arr;
            }
            return std::nullopt;
        }

        if (check(TokenKind::leftBrace))
        {
            auto obj = parseObject();
            if (obj.has_value())
            {
                return std::make_shared<AstObject>(std::move(*obj));
            }
            return std::nullopt;
        }

        addDiagnostic(
            DiagnosticSeverity::error,
            "expected value",
            peek().location
        );
        return std::nullopt;
    }

    std::optional<AstArray> Parser::parseArray()
    {
        const Token startTok = advance(); // consume '['
        AstArray arr{.elements = {}, .location = startTok.location};

        if (match(TokenKind::rightBracket))
        {
            return arr;
        }

        while (!isAtEnd())
        {
            auto val = parseValue();
            if (val.has_value())
            {
                arr.elements.push_back(std::move(*val));
            }
            else
            {
                synchronize();
            }

            if (match(TokenKind::comma))
            {
                if (check(TokenKind::rightBracket))
                {
                    // Allow trailing comma in array
                    advance();
                    break;
                }
                continue;
            }

            if (match(TokenKind::rightBracket))
            {
                break;
            }

            addDiagnostic(
                DiagnosticSeverity::error,
                "expected ',' or ']'",
                peek().location
            );
            synchronize();
            break;
        }

        return arr;
    }

    std::optional<AstCall> Parser::parseCall(const Token& identifierToken)
    {
        const SourceLocation loc = identifierToken.location;
        const std::string name = std::string(identifierToken.lexeme);

        advance(); // consume '('
        AstCall call{.name = name, .arguments = {}, .location = loc};

        if (match(TokenKind::rightParenthesis))
        {
            return call;
        }

        while (!isAtEnd())
        {
            auto val = parseValue();
            if (val.has_value())
            {
                call.arguments.push_back(std::move(*val));
            }
            else
            {
                synchronize();
            }

            if (match(TokenKind::comma))
            {
                if (check(TokenKind::rightParenthesis))
                {
                    advance();
                    break;
                }
                continue;
            }

            if (match(TokenKind::rightParenthesis))
            {
                break;
            }

            addDiagnostic(
                DiagnosticSeverity::error,
                "expected ',' or ')' in call arguments",
                peek().location
            );
            synchronize();
            break;
        }

        return call;
    }
}
