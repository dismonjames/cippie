#include <cippie/toolchain/ToolchainDetector.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Default detection should succeed
    {
        cippie::ToolchainDetector detector;
        auto res = detector.detect();
        assert(res.has_value());
        const auto& tc = *res;
        assert(!tc.cxxCompiler.empty());
        assert(!tc.name.empty());
        std::cout << "Toolchain: " << tc.name << " (" << tc.cxxCompiler.string() << ")\n";
    }

    // Host triple is valid
    {
        cippie::ToolchainDetector detector;
        auto res = detector.detect();
        assert(res.has_value());
        const auto& tc = *res;
        assert(tc.host.arch != cippie::Arch::unknown);
        assert(tc.host.os != cippie::Os::unknown);
        std::cout << "Host: " << tc.host.toString() << "\n";
    }

    // Native target == host
    {
        cippie::ToolchainDetector detector;
        auto res = detector.detect();
        assert(res.has_value());
        const auto& tc = *res;
        assert(tc.host == tc.target);
    }

    // Unknown toolchain name fails
    {
        cippie::ToolchainDetector detector;
        auto res = detector.detect(cippie::DetectOptions{
            .toolchainName = "non-existent-toolchain-99999"
        });
        assert(!res.has_value());
        std::cout << "Expected error: " << res.error().message << "\n";
    }

    // Cross target not natively runnable on x86_64 host
    {
        auto target = cippie::TargetTriple::parse("aarch64-linux-gnu");
        auto host = cippie::TargetTriple::detectHost();
        assert(target.has_value());

        if (host.arch != cippie::Arch::aarch64)
        {
            assert(!target->isNativeRunnable(host));
            std::cout << "Cross target aarch64 not runnable on " << host.toString() << " (correct)\n";
        }
        else
        {
            assert(target->isNativeRunnable(host));
            std::cout << "aarch64 target is natively runnable on aarch64 host (correct)\n";
        }
    }

    std::cout << "All ToolchainDetector tests passed\n";
    return 0;
}
