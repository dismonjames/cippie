#include <cippie/cache/FileHasher.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "cippie-file-hasher-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    const auto file1 = root / "file1.txt";
    const auto file2 = root / "file2.txt";

    std::ofstream(file1) << "Hello Cippie Hasher!";
    std::ofstream(file2) << "Hello Cippie Hasher!";

    cippie::FileHasher hasher;
    auto h1Res = hasher.hashFile(file1);
    auto h2Res = hasher.hashFile(file2);

    assert(h1Res.has_value());
    assert(h2Res.has_value());
    assert(*h1Res == *h2Res);
    assert(h1Res->size() == 16);

    // Modify file1 content
    std::ofstream(file1) << "Modified Content!";
    auto h3Res = hasher.hashFile(file1);
    assert(h3Res.has_value());
    assert(*h3Res != *h1Res);

    std::filesystem::remove_all(root);
    std::cout << "All FileHasher tests passed!\n";
    return 0;
}
