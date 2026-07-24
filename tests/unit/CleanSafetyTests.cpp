#include <cippie/util/CleanRunner.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-clean-safety-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / ".cippie/build");

    const auto safeBuildDir = root / ".cippie/build";

    // Test 1: Valid build directory is safe
    assert(cippie::CleanRunner::isSafeToDelete(safeBuildDir, root));

    // Test 2: Root path is UNSAFE
    assert(!cippie::CleanRunner::isSafeToDelete("/", root));

    // Test 3: Project root itself is UNSAFE
    assert(!cippie::CleanRunner::isSafeToDelete(root, root));

    // Test 4: System directories are UNSAFE
    assert(!cippie::CleanRunner::isSafeToDelete("/home", root));
    assert(!cippie::CleanRunner::isSafeToDelete("/usr", root));

    std::filesystem::remove_all(root);
    std::cout << "All CleanSafety tests passed!\n";
    return 0;
}
