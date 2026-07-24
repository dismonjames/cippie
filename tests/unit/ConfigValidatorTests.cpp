#include <cippie/config/ConfigLoader.hpp>
#include <cippie/config/Lexer.hpp>
#include <cippie/config/Parser.hpp>
#include <cippie/config/Validator.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

namespace
{
    std::filesystem::path findFixturesDir()
    {
        auto curr = std::filesystem::current_path();
        if (std::filesystem::exists(curr / "tests/fixtures"))
        {
            return curr / "tests/fixtures";
        }
        if (std::filesystem::exists(curr.parent_path() / "tests/fixtures"))
        {
            return curr.parent_path() / "tests/fixtures";
        }
        return curr / "tests/fixtures";
    }
}

int main()
{
    const std::filesystem::path fixturesDir = findFixturesDir();

    // Test 1: Valid single executable
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "single-executable/Cippiefile");
        assert(res.has_value());
        assert(res->name == "single-app");
        assert(res->cppStandard == 23);
        assert(res->targets.size() == 1);
        assert(res->targets[0].name == "app");
        assert(res->targets[0].entry.has_value());
    }

    // Test 2: Valid multiple targets
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "multiple-targets/Cippiefile");
        assert(res.has_value());
        assert(res->name == "multi-app");
        assert(res->targets.size() == 3);
        assert(res->defaultTarget.has_value() && *res->defaultTarget == "client");
        assert(res->dependencies.size() == 1);
        assert(res->dependencies[0].name == "fmt");
    }

    // Test 3: Invalid duplicate target
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "invalid-config/duplicate-target/Cippiefile");
        assert(!res.has_value());
    }

    // Test 4: Invalid dependency cycle
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "invalid-config/dependency-cycle/Cippiefile");
        assert(!res.has_value());
    }

    // Test 5: Invalid missing executable entry
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "invalid-config/missing-entry/Cippiefile");
        assert(!res.has_value());
    }

    // Test 6: Invalid unknown field
    {
        cippie::ConfigLoader loader;
        auto res = loader.loadFromFile(fixturesDir / "invalid-config/unknown-field/Cippiefile");
        assert(!res.has_value());
    }

    std::cout << "All Validator tests passed!\n";
    return 0;
}
