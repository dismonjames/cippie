#include <cippie/toolchain/TargetTriple.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // detectHost returns valid result
    {
        auto host = cippie::TargetTriple::detectHost();
        assert(host.arch != cippie::Arch::unknown);
        assert(host.os != cippie::Os::unknown);
        assert(!host.toString().empty());
        std::cout << "Host triple: " << host.toString() << "\n";
    }

    // Parse x86_64-linux-gnu
    {
        auto res = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(res.has_value());
        assert(res->arch == cippie::Arch::x86_64);
        assert(res->os == cippie::Os::linux_);
        assert(res->abi == cippie::Abi::gnu);
    }

    // Parse aarch64-linux-gnu
    {
        auto res = cippie::TargetTriple::parse("aarch64-linux-gnu");
        assert(res.has_value());
        assert(res->arch == cippie::Arch::aarch64);
        assert(res->os == cippie::Os::linux_);
        assert(res->abi == cippie::Abi::gnu);
    }

    // Parse x86_64-linux-musl
    {
        auto res = cippie::TargetTriple::parse("x86_64-linux-musl");
        assert(res.has_value());
        assert(res->arch == cippie::Arch::x86_64);
        assert(res->os == cippie::Os::linux_);
        assert(res->abi == cippie::Abi::musl);
    }

    // Parse x86_64-windows-gnu
    {
        auto res = cippie::TargetTriple::parse("x86_64-windows-gnu");
        assert(res.has_value());
        assert(res->arch == cippie::Arch::x86_64);
        assert(res->os == cippie::Os::windows);
    }

    // Parse four-part triple x86_64-pc-linux-gnu
    {
        auto res = cippie::TargetTriple::parse("x86_64-pc-linux-gnu");
        assert(res.has_value());
        assert(res->arch == cippie::Arch::x86_64);
        assert(res->os == cippie::Os::linux_);
        assert(res->abi == cippie::Abi::gnu);
    }

    // Reject unknown arch
    {
        auto res = cippie::TargetTriple::parse("mips64-linux-gnu");
        assert(!res.has_value());
        assert(res.error().code == cippie::ErrorCode::validationFailed);
    }

    // Reject unknown OS
    {
        auto res = cippie::TargetTriple::parse("x86_64-zos-gnu");
        assert(!res.has_value());
        assert(res.error().code == cippie::ErrorCode::validationFailed);
    }

    // Canonical toString
    {
        auto res = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(res.has_value());
        assert(res->toString() == "x86_64-linux-gnu");
    }

    // Equality
    {
        auto r1 = cippie::TargetTriple::parse("x86_64-linux-gnu");
        auto r2 = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(r1.has_value());
        assert(r2.has_value());
        assert(*r1 == *r2);
    }

    // isNativeRunnable same arch
    {
        auto target = cippie::TargetTriple::parse("x86_64-linux-gnu");
        auto host = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(target.has_value() && host.has_value());
        assert(target->isNativeRunnable(*host));
    }

    // isNativeRunnable different arch
    {
        auto target = cippie::TargetTriple::parse("aarch64-linux-gnu");
        auto host = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(target.has_value() && host.has_value());
        assert(!target->isNativeRunnable(*host));
    }

    // isNativeRunnable different OS
    {
        auto target = cippie::TargetTriple::parse("x86_64-windows-gnu");
        auto host = cippie::TargetTriple::parse("x86_64-linux-gnu");
        assert(target.has_value() && host.has_value());
        assert(!target->isNativeRunnable(*host));
    }

    // Empty string returns host
    {
        auto res = cippie::TargetTriple::parse("");
        assert(res.has_value());
        auto host = cippie::TargetTriple::detectHost();
        assert(res->arch == host.arch);
        assert(res->os == host.os);
    }

    std::cout << "All TargetTriple tests passed\n";
    return 0;
}
