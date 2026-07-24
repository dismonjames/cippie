#include <cippie/util/SHA256.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Test 1: Empty string SHA-256
    std::string h1 = cippie::SHA256::hashString("");
    assert(h1 == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // Test 2: Known test vector "abc"
    std::string h2 = cippie::SHA256::hashString("abc");
    assert(h2 == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    std::cout << "All SHA256 tests passed!\n";
    return 0;
}
