#include <cippie/cache/DepFileParser.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Test 1: Simple depfile parsing with line continuation and escaped spaces
    {
        std::string content =
            "build/obj/App.o: src/App.cpp \\\n"
            "  include/App.hpp \\\n"
            "  include/my\\ header.hpp\n"
            "\n"
            "include/App.hpp:\n"
            "include/my\\ header.hpp:\n";

        auto deps = cippie::DepFileParser::parseString(content);
        assert(deps.size() == 3); // src/App.cpp, include/App.hpp, include/my header.hpp
        assert(deps[0] == std::filesystem::path("include/App.hpp").lexically_normal());
        assert(deps[1] == std::filesystem::path("include/my header.hpp").lexically_normal());
        assert(deps[2] == std::filesystem::path("src/App.cpp").lexically_normal());
    }

    // Test 2: Malformed content returns parsed entries safely without crash
    {
        std::string malformed = "invalid_line_without_colon\n:::\n";
        auto deps = cippie::DepFileParser::parseString(malformed);
        // Does not crash
    }

    std::cout << "All DepFileParser tests passed!\n";
    return 0;
}
