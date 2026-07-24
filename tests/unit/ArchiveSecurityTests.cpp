#include <cippie/util/ArchiveExtractor.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "cippie-archive-sec-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    // Test 1: Normal path inside extraction root is safe
    const auto safePath = root / "src/main.cpp";
    assert(cippie::ArchiveExtractor::isPathSafe(safePath, root));

    // Test 2: Escalating path outside root is rejected
    const auto unsafePath = root / "../outside.txt";
    assert(!cippie::ArchiveExtractor::isPathSafe(unsafePath, root));

    std::filesystem::remove_all(root);
    std::cout << "All ArchiveSecurity tests passed!\n";
    return 0;
}
