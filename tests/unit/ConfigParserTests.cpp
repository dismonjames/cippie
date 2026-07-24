#include <cippie/config/Lexer.hpp>
#include <cippie/config/Parser.hpp>

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    // Test 1: Minimal project
    {
        std::string source = R"(
            project("hello") {
                cpp = 23;
                executable("app") {
                    entry = "src/main.cpp";
                }
            }
        )";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();
        assert(!lexer.hasErrors());

        cippie::Parser parser(tokens, "test.cippie");
        auto ast = parser.parseProject();

        assert(!parser.hasErrors());
        assert(ast.has_value());
        assert(ast->name == "hello");
        assert(ast->body.statements.size() == 2);
    }

    // Test 2: Multiple targets and calls
    {
        std::string source = R"(
            project("suite") {
                dependencies = [
                    package("fmt", "11.2.0")
                ];
                library("core") {
                    type = static;
                }
                executable("client") {
                    entry = "apps/client/main.cpp";
                    links = ["core", dependency("fmt")];
                }
            }
        )";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();
        cippie::Parser parser(tokens, "test.cippie");
        auto ast = parser.parseProject();

        assert(!parser.hasErrors());
        assert(ast.has_value());
        assert(ast->name == "suite");
    }

    // Test 3: Malformed syntax recovery
    {
        std::string source = R"(
            project("bad") {
                sources = ["a.cpp" "b.cpp"];
                cpp = 23;
            }
        )";
        cippie::Lexer lexer(source, "test.cippie");
        auto tokens = lexer.tokenize();
        cippie::Parser parser(tokens, "test.cippie");
        auto ast = parser.parseProject();

        assert(parser.hasErrors());
        // Diagnostic contained "expected ',' or ']'"
    }

    std::cout << "All Parser tests passed!\n";
    return 0;
}
