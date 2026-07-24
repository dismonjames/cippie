#include <cippie/build/SourceScanner.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-source-scanner-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "src/core");
    std::filesystem::create_directories(root / "src/utils");
    std::filesystem::create_directories(root / ".git");
    std::filesystem::create_directories(root / ".cippie");
    std::filesystem::create_directories(root / "build");
    std::filesystem::create_directories(root / "cmake-build-debug");

    std::ofstream(root / "src/main.cpp") << "int main() {}";
    std::ofstream(root / "src/core/app.cpp") << "void app() {}";
    std::ofstream(root / "src/core/helper.cc") << "void helper() {}";
    std::ofstream(root / "src/utils/math.cxx") << "void math() {}";

    // Files in ignored directories
    std::ofstream(root / ".git/git.cpp") << "ignored";
    std::ofstream(root / ".cippie/cache.cpp") << "ignored";
    std::ofstream(root / "build/built.cpp") << "ignored";
    std::ofstream(root / "cmake-build-debug/temp.cpp") << "ignored";

    cippie::SourceScanner scanner;

    // Test 1: Recursive pattern scanning
    {
        auto sources = scanner.scan(root, {"src/**/*.cpp", "src/**/*.cc", "src/**/*.cxx"});
        assert(sources.size() == 4);
        assert(sources[0] == (root / "src/core/app.cpp").lexically_normal());
        assert(sources[1] == (root / "src/core/helper.cc").lexically_normal());
        assert(sources[2] == (root / "src/main.cpp").lexically_normal());
        assert(sources[3] == (root / "src/utils/math.cxx").lexically_normal());
    }

    // Test 2: Sorting and deduplication
    {
        auto sources = scanner.scan(root, {"src/main.cpp", "src/main.cpp", "src/core/*.cpp"});
        assert(sources.size() == 2); // main.cpp + app.cpp
    }

    std::filesystem::remove_all(root);
    std::cout << "All SourceScanner tests passed!\n";
    return 0;
}
