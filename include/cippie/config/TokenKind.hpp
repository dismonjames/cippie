#pragma once

#include <string_view>

namespace cippie
{
    enum class TokenKind
    {
        identifier,
        stringLiteral,
        integerLiteral,
        trueKeyword,
        falseKeyword,

        projectKeyword,
        executableKeyword,
        libraryKeyword,
        staticLibraryKeyword,
        sharedLibraryKeyword,
        testKeyword,
        packageKeyword,
        dependencyKeyword,

        leftParenthesis,
        rightParenthesis,
        leftBrace,
        rightBrace,
        leftBracket,
        rightBracket,

        comma,
        semicolon,
        equal,

        endOfFile,
        invalid
    };

    [[nodiscard]] constexpr std::string_view toString(TokenKind kind) noexcept
    {
        switch (kind)
        {
            case TokenKind::identifier:
                return "identifier";
            case TokenKind::stringLiteral:
                return "stringLiteral";
            case TokenKind::integerLiteral:
                return "integerLiteral";
            case TokenKind::trueKeyword:
                return "true";
            case TokenKind::falseKeyword:
                return "false";
            case TokenKind::projectKeyword:
                return "project";
            case TokenKind::executableKeyword:
                return "executable";
            case TokenKind::libraryKeyword:
                return "library";
            case TokenKind::staticLibraryKeyword:
                return "static_library";
            case TokenKind::sharedLibraryKeyword:
                return "shared_library";
            case TokenKind::testKeyword:
                return "test";
            case TokenKind::packageKeyword:
                return "package";
            case TokenKind::dependencyKeyword:
                return "dependency";
            case TokenKind::leftParenthesis:
                return "(";
            case TokenKind::rightParenthesis:
                return ")";
            case TokenKind::leftBrace:
                return "{";
            case TokenKind::rightBrace:
                return "}";
            case TokenKind::leftBracket:
                return "[";
            case TokenKind::rightBracket:
                return "]";
            case TokenKind::comma:
                return ",";
            case TokenKind::semicolon:
                return ";";
            case TokenKind::equal:
                return "=";
            case TokenKind::endOfFile:
                return "endOfFile";
            case TokenKind::invalid:
                return "invalid";
        }
        return "invalid";
    }
}
