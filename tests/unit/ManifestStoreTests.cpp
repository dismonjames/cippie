#include <cippie/cache/ManifestStore.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "cippie-manifest-store-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    const auto manifestPath = root / "manifest.txt";
    cippie::ManifestStore store;

    // Test 1: Save & Load Roundtrip
    {
        cippie::CacheEntry entry1{
            .targetName = "client",
            .sourcePath = root / "src/main.cpp",
            .objectPath = root / "obj/main.cpp.o",
            .depFilePath = root / "dep/main.cpp.d",
            .cacheKey = "1234567890abcdef",
            .dependencies = {{root / "include/app.hpp", "abcdef1234567890"}}
        };

        auto saveRes = store.save(manifestPath, {entry1});
        assert(saveRes.has_value());

        auto loaded = store.load(manifestPath);
        assert(loaded.size() == 1);
        assert(loaded[0].targetName == "client");
        assert(loaded[0].cacheKey == "1234567890abcdef");
        assert(loaded[0].dependencies.size() == 1);
        assert(loaded[0].dependencies[0].second == "abcdef1234567890");
    }

    // Test 2: Corrupted Manifest Recovery (returns empty manifest without crash)
    {
        std::ofstream file(manifestPath);
        file << "CORRUPTED_LINE_TAG_123 456 789\n";
        file.close();

        auto loaded = store.load(manifestPath);
        assert(loaded.empty());
    }

    std::filesystem::remove_all(root);
    std::cout << "All ManifestStore tests passed!\n";
    return 0;
}
