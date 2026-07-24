#include <cippie/package/VersionRequirement.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Test 1: Caret ^1.2.3  => >=1.2.3 <2.0.0
    auto req1 = cippie::VersionRequirement::parse("^1.2.3").value();
    assert(req1.matches(cippie::SemanticVersion::parse("1.2.3").value()));
    assert(req1.matches(cippie::SemanticVersion::parse("1.9.9").value()));
    assert(!req1.matches(cippie::SemanticVersion::parse("2.0.0").value()));
    assert(!req1.matches(cippie::SemanticVersion::parse("1.2.2").value()));

    // Test 2: Caret 0.x (^0.2.3 => >=0.2.3 <0.3.0)
    auto req2 = cippie::VersionRequirement::parse("^0.2.3").value();
    assert(req2.matches(cippie::SemanticVersion::parse("0.2.3").value()));
    assert(req2.matches(cippie::SemanticVersion::parse("0.2.9").value()));
    assert(!req2.matches(cippie::SemanticVersion::parse("0.3.0").value()));

    // Test 3: Caret 0.0.x (^0.0.3 => >=0.0.3 <0.0.4)
    auto req3 = cippie::VersionRequirement::parse("^0.0.3").value();
    assert(req3.matches(cippie::SemanticVersion::parse("0.0.3").value()));
    assert(!req3.matches(cippie::SemanticVersion::parse("0.0.4").value()));

    // Test 4: Tilde ~1.2.3 => >=1.2.3 <1.3.0
    auto req4 = cippie::VersionRequirement::parse("~1.2.3").value();
    assert(req4.matches(cippie::SemanticVersion::parse("1.2.5").value()));
    assert(!req4.matches(cippie::SemanticVersion::parse("1.3.0").value()));

    // Test 5: Conjunctions (>=1.2.0 <2.0.0)
    auto req5 = cippie::VersionRequirement::parse(">=1.2.0 <2.0.0").value();
    assert(req5.matches(cippie::SemanticVersion::parse("1.5.0").value()));
    assert(!req5.matches(cippie::SemanticVersion::parse("2.0.0").value()));

    std::cout << "All VersionRequirement tests passed!\n";
    return 0;
}
