#include <cippie/package/SemanticVersion.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Test 1: Parse valid stable version
    auto v1Res = cippie::SemanticVersion::parse("1.2.3");
    assert(v1Res.has_value());
    assert(v1Res->major == 1 && v1Res->minor == 2 && v1Res->patch == 3);
    assert(v1Res->toString() == "1.2.3");

    // Test 2: Parse prerelease & build metadata
    auto v2Res = cippie::SemanticVersion::parse("1.2.3-alpha.1+build.7");
    assert(v2Res.has_value());
    assert(v2Res->isPrerelease());
    assert(v2Res->toString() == "1.2.3-alpha.1+build.7");

    // Test 3: Reject missing component (e.g. 1.2)
    auto invalid1 = cippie::SemanticVersion::parse("1.2");
    assert(!invalid1.has_value());

    // Test 4: Reject leading zeroes (e.g. 01.2.3)
    auto invalid2 = cippie::SemanticVersion::parse("01.2.3");
    assert(!invalid2.has_value());

    // Test 5: Precedence comparisons
    auto vA = cippie::SemanticVersion::parse("1.0.0-alpha").value();
    auto vB = cippie::SemanticVersion::parse("1.0.0-alpha.1").value();
    auto vC = cippie::SemanticVersion::parse("1.0.0").value();
    auto vD = cippie::SemanticVersion::parse("2.0.0").value();

    assert(vA < vB);
    assert(vB < vC);
    assert(vC < vD);

    std::cout << "All SemanticVersion tests passed!\n";
    return 0;
}
