#pragma once

#include <cippie/core/Result.hpp>
#include <string>
#include <string_view>

namespace cippie
{
    enum class Arch
    {
        x86_64,
        aarch64,
        arm,
        i686,
        riscv64,
        unknown
    };

    enum class Os
    {
        linux_,
        windows,
        macos,
        none,
        unknown
    };

    enum class Abi
    {
        gnu,
        musl,
        msvc,
        mingw,
        darwin,
        none,
        unknown
    };

    struct TargetTriple
    {
        Arch arch{Arch::x86_64};
        std::string vendor{"unknown"};
        Os os{Os::linux_};
        Abi abi{Abi::gnu};

        [[nodiscard]] std::string toString() const;

        // Returns false if target cannot run natively on host (cross target)
        [[nodiscard]] bool isNativeRunnable(const TargetTriple& host) const noexcept;

        [[nodiscard]] static Result<TargetTriple> parse(const std::string& s);
        [[nodiscard]] static TargetTriple detectHost() noexcept;

        [[nodiscard]] bool operator==(const TargetTriple& other) const noexcept = default;

        [[nodiscard]] static std::string_view archName(Arch a) noexcept;
        [[nodiscard]] static std::string_view osName(Os o) noexcept;
        [[nodiscard]] static std::string_view abiName(Abi a) noexcept;
    };
}
