#include <cippie/config/Lexer.hpp>

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    // Test 1: Identifiers and keywords
    {
        std::string source = "project executable library test package dependency true false my_target core-1";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(!lexer.hasErrors());
        assert(tokens.size() == 11); // 10 tokens + EOF
        assert(tokens[0].kind == cippie::TokenKind::projectKeyword);
        assert(tokens[1].kind == cippie::TokenKind::executableKeyword);
        assert(tokens[2].kind == cippie::TokenKind::libraryKeyword);
        assert(tokens[3].kind == cippie::TokenKind::testKeyword);
        assert(tokens[4].kind == cippie::TokenKind::packageKeyword);
        assert(tokens[5].kind == cippie::TokenKind::dependencyKeyword);
        assert(tokens[6].kind == cippie::TokenKind::trueKeyword);
        assert(tokens[7].kind == cippie::TokenKind::falseKeyword);
        assert(tokens[8].kind == cippie::TokenKind::identifier);
        assert(tokens[8].lexeme == "my_target");
        assert(tokens[9].kind == cippie::TokenKind::identifier);
        assert(tokens[9].lexeme == "core-1");
    }

    // Test 2: Strings and escapes
    {
        std::string source = R"("hello world" "escaped \"quotes\" and \n newline")";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(!lexer.hasErrors());
        assert(tokens.size() == 3); // 2 strings + EOF
        assert(tokens[0].kind == cippie::TokenKind::stringLiteral);
        assert(cippie::Lexer::unescapeString(tokens[0].lexeme) == "hello world");
        assert(tokens[1].kind == cippie::TokenKind::stringLiteral);
        assert(cippie::Lexer::unescapeString(tokens[1].lexeme) == "escaped \"quotes\" and \n newline");
    }

    // Test 3: Integers and delimiters
    {
        std::string source = "cpp = 23; ( ) { } [ ] ,";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(!lexer.hasErrors());
        assert(tokens[0].kind == cippie::TokenKind::identifier);
        assert(tokens[1].kind == cippie::TokenKind::equal);
        assert(tokens[2].kind == cippie::TokenKind::integerLiteral);
        assert(tokens[2].lexeme == "23");
        assert(tokens[3].kind == cippie::TokenKind::semicolon);
    }

    // Test 4: Line and block comments
    {
        std::string source = R"(
            // Line comment
            project("test") /* Block
                               comment */ {
                cpp = 23;
            }
        )";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(!lexer.hasErrors());
        assert(tokens[0].kind == cippie::TokenKind::projectKeyword);
    }

    // Test 5: Unterminated string
    {
        std::string source = R"("unterminated)";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(lexer.hasErrors());
    }

    // Test 6: Unterminated block comment
    {
        std::string source = "/* unclosed comment";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(lexer.hasErrors());
    }

    // Test 7: Source location tracking
    {
        std::string source = "first\nsecond";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();

        assert(tokens[0].location.line == 1);
        assert(tokens[0].location.column == 1);
        assert(tokens[1].location.line == 2);
        assert(tokens[1].location.column == 1);
    }

    std::cout << "All Lexer tests passed!\n";
    return 0;
}
