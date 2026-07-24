#include <cippie/config/Lexer.hpp>

#include <cctype>
#include <sstream>

namespace cippie
{
    Lexer::Lexer(std::string source, std::filesystem::path filePath)
        : m_source(std::move(source))
        , m_filePath(std::move(filePath))
    {
    }

    char Lexer::peek() const noexcept
    {
        if (isAtEnd())
        {
            return '\0';
        }
        return m_source[m_current];
    }

    char Lexer::peekNext() const noexcept
    {
        if (m_current + 1 >= m_source.size())
        {
            return '\0';
        }
        return m_source[m_current + 1];
    }

    char Lexer::advance() noexcept
    {
        if (isAtEnd())
        {
            return '\0';
        }

        const char c = m_source[m_current++];

        if (c == '\n')
        {
            m_line++;
            m_column = 1;
        }
        else if ((static_cast<unsigned char>(c) & 0xC0) != 0x80)
        {
            m_column++;
        }

        return c;
    }

    bool Lexer::isAtEnd() const noexcept
    {
        return m_current >= m_source.size();
    }

    void Lexer::addToken(TokenKind kind)
    {
        const std::string_view lexeme(
            m_source.data() + m_start,
            m_current - m_start
        );

        SourceLocation loc;
        loc.file = m_filePath;
        loc.line = m_line;
        loc.column = m_startColumn;
        loc.offset = m_start;

        m_tokens.push_back(Token{
            .kind = kind,
            .lexeme = lexeme,
            .location = loc
        });
    }

    void Lexer::addDiagnostic(
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

    std::vector<Token> Lexer::tokenize()
    {
        m_tokens.clear();
        m_diagnostics.clear();
        m_start = 0;
        m_current = 0;
        m_line = 1;
        m_column = 1;

        while (!isAtEnd())
        {
            skipWhitespaceAndComments();
            if (isAtEnd())
            {
                break;
            }

            m_start = m_current;
            m_startColumn = m_column;

            scanToken();
        }

        SourceLocation eofLoc;
        eofLoc.file = m_filePath;
        eofLoc.line = m_line;
        eofLoc.column = m_column;
        eofLoc.offset = m_current;

        m_tokens.push_back(Token{
            .kind = TokenKind::endOfFile,
            .lexeme = {},
            .location = eofLoc
        });

        return m_tokens;
    }

    const std::vector<Diagnostic>& Lexer::diagnostics() const noexcept
    {
        return m_diagnostics;
    }

    bool Lexer::hasErrors() const noexcept
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

    void Lexer::skipWhitespaceAndComments()
    {
        while (!isAtEnd())
        {
            const char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                advance();
            }
            else if (c == '/' && peekNext() == '/')
            {
                // Line comment
                advance(); // '/'
                advance(); // '/'
                while (!isAtEnd() && peek() != '\n')
                {
                    advance();
                }
            }
            else if (c == '/' && peekNext() == '*')
            {
                // Block comment
                const std::size_t startLine = m_line;
                const std::size_t startCol = m_column;
                const std::size_t startOffset = m_current;

                advance(); // '/'
                advance(); // '*'

                bool closed = false;
                while (!isAtEnd())
                {
                    if (peek() == '*' && peekNext() == '/')
                    {
                        advance(); // '*'
                        advance(); // '/'
                        closed = true;
                        break;
                    }
                    advance();
                }

                if (!closed)
                {
                    SourceLocation loc;
                    loc.file = m_filePath;
                    loc.line = startLine;
                    loc.column = startCol;
                    loc.offset = startOffset;
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "unterminated block comment",
                        loc
                    );
                }
            }
            else
            {
                break;
            }
        }
    }

    void Lexer::scanToken()
    {
        const char c = advance();

        switch (c)
        {
            case '(':
                addToken(TokenKind::leftParenthesis);
                break;
            case ')':
                addToken(TokenKind::rightParenthesis);
                break;
            case '{':
                addToken(TokenKind::leftBrace);
                break;
            case '}':
                addToken(TokenKind::rightBrace);
                break;
            case '[':
                addToken(TokenKind::leftBracket);
                break;
            case ']':
                addToken(TokenKind::rightBracket);
                break;
            case ',':
                addToken(TokenKind::comma);
                break;
            case ';':
                addToken(TokenKind::semicolon);
                break;
            case '=':
                addToken(TokenKind::equal);
                break;
            case '"':
                scanString();
                break;

            default:
                if (std::isdigit(static_cast<unsigned char>(c)))
                {
                    scanNumber();
                }
                else if (std::isalpha(static_cast<unsigned char>(c)) ||
                         c == '_' || c == '-')
                {
                    scanIdentifierOrKeyword();
                }
                else
                {
                    SourceLocation loc;
                    loc.file = m_filePath;
                    loc.line = m_line;
                    loc.column = m_startColumn;
                    loc.offset = m_start;

                    std::string msg = "unexpected character '";
                    msg += c;
                    msg += "'";
                    addDiagnostic(DiagnosticSeverity::error, msg, loc);
                    addToken(TokenKind::invalid);
                }
                break;
        }
    }

    void Lexer::scanIdentifierOrKeyword()
    {
        while (!isAtEnd())
        {
            const char c = peek();
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')
            {
                advance();
            }
            else
            {
                break;
            }
        }

        const std::string_view text(
            m_source.data() + m_start,
            m_current - m_start
        );

        if (text == "project")
        {
            addToken(TokenKind::projectKeyword);
        }
        else if (text == "executable")
        {
            addToken(TokenKind::executableKeyword);
        }
        else if (text == "library")
        {
            addToken(TokenKind::libraryKeyword);
        }
        else if (text == "test")
        {
            addToken(TokenKind::testKeyword);
        }
        else if (text == "package")
        {
            addToken(TokenKind::packageKeyword);
        }
        else if (text == "dependency")
        {
            addToken(TokenKind::dependencyKeyword);
        }
        else if (text == "true")
        {
            addToken(TokenKind::trueKeyword);
        }
        else if (text == "false")
        {
            addToken(TokenKind::falseKeyword);
        }
        else
        {
            addToken(TokenKind::identifier);
        }
    }

    void Lexer::scanNumber()
    {
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
        {
            advance();
        }
        addToken(TokenKind::integerLiteral);
    }

    void Lexer::scanString()
    {
        bool terminated = false;
        while (!isAtEnd())
        {
            const char c = peek();
            if (c == '"')
            {
                advance();
                terminated = true;
                break;
            }
            if (c == '\\')
            {
                advance();
                if (!isAtEnd())
                {
                    advance();
                }
            }
            else if (c == '\n')
            {
                // Multi-line string without escape is unterminated string error
                break;
            }
            else
            {
                advance();
            }
        }

        if (!terminated)
        {
            SourceLocation loc;
            loc.file = m_filePath;
            loc.line = m_line;
            loc.column = m_startColumn;
            loc.offset = m_start;
            addDiagnostic(
                DiagnosticSeverity::error,
                "unterminated string literal",
                loc
            );
            addToken(TokenKind::invalid);
            return;
        }

        addToken(TokenKind::stringLiteral);
    }

    std::string Lexer::unescapeString(
        std::string_view rawWithQuotes,
        bool* hasError
    )
    {
        if (hasError != nullptr)
        {
            *hasError = false;
        }

        if (rawWithQuotes.size() < 2 || rawWithQuotes.front() != '"' ||
            rawWithQuotes.back() != '"')
        {
            return std::string(rawWithQuotes);
        }

        const std::string_view inner = rawWithQuotes.substr(
            1,
            rawWithQuotes.size() - 2
        );
        std::string result;
        result.reserve(inner.size());

        for (std::size_t i = 0; i < inner.size(); ++i)
        {
            if (inner[i] == '\\')
            {
                if (i + 1 >= inner.size())
                {
                    if (hasError != nullptr)
                    {
                        *hasError = true;
                    }
                    result.push_back('\\');
                    break;
                }
                const char next = inner[++i];
                switch (next)
                {
                    case '"':
                        result.push_back('"');
                        break;
                    case '\\':
                        result.push_back('\\');
                        break;
                    case 'n':
                        result.push_back('\n');
                        break;
                    case 't':
                        result.push_back('\t');
                        break;
                    case 'r':
                        result.push_back('\r');
                        break;
                    default:
                        if (hasError != nullptr)
                        {
                            *hasError = true;
                        }
                        result.push_back(next);
                        break;
                }
            }
            else
            {
                result.push_back(inner[i]);
            }
        }

        return result;
    }
}
