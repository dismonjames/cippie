#include <cippie/build/SourceScanner.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-source-scanner-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "nested");

    std::ofstream(root / "main.cpp") << "int main() {}";
    std::ofstream(root / "nested/tool.cpp") << "void tool() {}";
    std::ofstream(root / "ignored.txt") << "ignored";

    cippie::SourceScanner scanner;
    const auto sources = scanner.scan(root);

    assert(sources.size() == 2);

    std::filesystem::remove_all(root);
    return 0;
}
