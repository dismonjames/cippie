#include <cippie/util/BuildLock.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "cippie-build-lock-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    // Test 1: Acquire lock
    auto lock1Res = cippie::BuildLock::acquire(root);
    assert(lock1Res.has_value());
    assert(lock1Res->isAcquired());

    // Test 2: Second acquire attempt fails non-blockingly
    auto lock2Res = cippie::BuildLock::acquire(root);
    assert(!lock2Res.has_value());
    assert(lock2Res.error().message.find("lock held") != std::string::npos);

    // Test 3: Release lock1 allows new lock acquisition
    lock1Res->release();
    auto lock3Res = cippie::BuildLock::acquire(root);
    assert(lock3Res.has_value());

    std::filesystem::remove_all(root);
    std::cout << "All BuildLock tests passed!\n";
    return 0;
}
