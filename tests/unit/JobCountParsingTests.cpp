#include <cippie/cli/CommandLine.hpp>

#include <cassert>
#include <iostream>

int main()
{
    cippie::CommandLineParser parser;

    // Test 1: -j 4
    {
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"-j", (char*)"4"};
        auto cl = parser.parse(4, argv, "/tmp");
        assert(cl.jobs == 4);
    }

    // Test 2: -j4
    {
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"-j4"};
        auto cl = parser.parse(3, argv, "/tmp");
        assert(cl.jobs == 4);
    }

    // Test 3: --jobs 8
    {
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"--jobs", (char*)"8"};
        auto cl = parser.parse(4, argv, "/tmp");
        assert(cl.jobs == 8);
    }

    // Test 4: --jobs=16
    {
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"--jobs=16"};
        auto cl = parser.parse(3, argv, "/tmp");
        assert(cl.jobs == 16);
    }

    // Test 5: Invalid/zero job count rejects command
    {
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"-j", (char*)"0"};
        auto cl = parser.parse(4, argv, "/tmp");
        assert(cl.type == cippie::CommandType::unknown);
    }

    std::cout << "All JobCountParsing tests passed!\n";
    return 0;
}
