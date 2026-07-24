#include <cippie/cache/CacheKey.hpp>

#include <cassert>
#include <iostream>

int main()
{
    const std::string compiler = "/usr/bin/clang++";
    const std::string version = "18.1.0";
    const std::string family = "clang";
    const std::string triple = "x86_64-linux-gnu";
    const std::string config = "debug";
    const int stdVer = 23;
    const bool pic = false;
    const std::filesystem::path source = "src/main.cpp";
    const std::string srcHash = "1234567890abcdef";
    const std::vector<std::filesystem::path> incs = {"include"};
    const std::vector<std::string> defs = {"DEBUG=1"};
    const std::vector<std::string> opts = {"-Wall"};
    const std::vector<std::pair<std::filesystem::path, std::string>> deps = {{"include/app.hpp", "abcdef1234567890"}};

    std::string baseKey = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, config, stdVer, pic, source, srcHash, incs, defs, opts, deps
    );

    // 1. Same parameters produce identical key
    std::string baseKey2 = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, config, stdVer, pic, source, srcHash, incs, defs, opts, deps
    );
    assert(baseKey == baseKey2);

    // 2. Source hash change invalidates key
    std::string modSrcKey = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, config, stdVer, pic, source, "changed_hash", incs, defs, opts, deps
    );
    assert(baseKey != modSrcKey);

    // 3. Header dependency hash change invalidates key
    auto modDeps = deps;
    modDeps[0].second = "header_changed_hash";
    std::string modDepKey = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, config, stdVer, pic, source, srcHash, incs, defs, opts, modDeps
    );
    assert(baseKey != modDepKey);

    // 4. Debug vs Release invalidates key
    std::string relKey = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, "release", stdVer, pic, source, srcHash, incs, defs, opts, deps
    );
    assert(baseKey != relKey);

    // 5. PIC mode change invalidates key
    std::string picKey = cippie::CacheKeyBuilder::buildCompileKey(
        compiler, version, family, triple, config, stdVer, true, source, srcHash, incs, defs, opts, deps
    );
    assert(baseKey != picKey);

    std::cout << "All CacheKey tests passed!\n";
    return 0;
}
