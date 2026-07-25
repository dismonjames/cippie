#include <cippie/cli/CommandLine.hpp>
#include <cippie/core/ExitCode.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    // Test 1: ExitCode enum mapping values
    assert(cippie::toInt(cippie::ExitCode::success) == 0);
    assert(cippie::toInt(cippie::ExitCode::generalError) == 1);
    assert(cippie::toInt(cippie::ExitCode::invalidArguments) == 2);
    assert(cippie::toInt(cippie::ExitCode::projectNotFound) == 3);
    assert(cippie::toInt(cippie::ExitCode::configurationError) == 4);
    assert(cippie::toInt(cippie::ExitCode::dependencyResolutionFailed) == 5);
    assert(cippie::toInt(cippie::ExitCode::buildFailed) == 6);
    assert(cippie::toInt(cippie::ExitCode::testFailed) == 7);
    assert(cippie::toInt(cippie::ExitCode::toolchainFailed) == 8);
    assert(cippie::toInt(cippie::ExitCode::packageFailed) == 9);
    assert(cippie::toInt(cippie::ExitCode::internalFailure) == 10);

    // Test 2: CLI parser handles --jobs 0 as unknown/invalid argument
    {
        cippie::CommandLineParser parser;
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"--jobs", (char*)"0", nullptr};
        auto cmd = parser.parse(4, argv, ".");
        assert(cmd.type == cippie::CommandType::unknown);
    }

    // Test 3: CLI parser handles --jobs without value at end of line as unknown
    {
        cippie::CommandLineParser parser;
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"--jobs", nullptr};
        auto cmd = parser.parse(3, argv, ".");
        assert(cmd.type == cippie::CommandType::unknown);
    }

    // Test 4: CLI parser handles unknown flags as unknown
    {
        cippie::CommandLineParser parser;
        char* argv[] = {(char*)"cippie", (char*)"build", (char*)"--unknown-flag-12345", nullptr};
        auto cmd = parser.parse(3, argv, ".");
        assert(cmd.type == cippie::CommandType::unknown);
    }

    // Test 5: CLI parser handles forwarded arguments after --
    {
        cippie::CommandLineParser parser;
        char* argv[] = {(char*)"cippie", (char*)"run", (char*)"app", (char*)"--", (char*)"--arg1", (char*)"val1", nullptr};
        auto cmd = parser.parse(6, argv, ".");
        assert(cmd.type == cippie::CommandType::run);
        assert(cmd.target == "app");
        assert(cmd.forwardedArguments.size() == 2);
        assert(cmd.forwardedArguments[0] == "--arg1");
        assert(cmd.forwardedArguments[1] == "val1");
    }

    std::cout << "ExitCode and CLI regression tests passed\n";
    return 0;
}
