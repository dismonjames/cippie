#include <cippie/toolchain/TargetTriple.hpp>

#include <sstream>
#include <vector>

#if defined(__x86_64__) || defined(_M_X64)
#define CIPPIE_HOST_ARCH_ENUM cippie::Arch::x86_64
#define CIPPIE_HOST_ARCH_STR "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define CIPPIE_HOST_ARCH_ENUM cippie::Arch::aarch64
#define CIPPIE_HOST_ARCH_STR "aarch64"
#elif defined(__i386__) || defined(_M_IX86)
#define CIPPIE_HOST_ARCH_ENUM cippie::Arch::i686
#define CIPPIE_HOST_ARCH_STR "i686"
#else
#define CIPPIE_HOST_ARCH_ENUM cippie::Arch::x86_64
#define CIPPIE_HOST_ARCH_STR "x86_64"
#endif

#if defined(__linux__)
#define CIPPIE_HOST_OS_ENUM cippie::Os::linux_
#define CIPPIE_HOST_OS_STR "linux"
#define CIPPIE_HOST_ABI_ENUM cippie::Abi::gnu
#define CIPPIE_HOST_ABI_STR "gnu"
#elif defined(_WIN32)
#define CIPPIE_HOST_OS_ENUM cippie::Os::windows
#define CIPPIE_HOST_OS_STR "windows"
#define CIPPIE_HOST_ABI_ENUM cippie::Abi::msvc
#define CIPPIE_HOST_ABI_STR "msvc"
#elif defined(__APPLE__)
#define CIPPIE_HOST_OS_ENUM cippie::Os::macos
#define CIPPIE_HOST_OS_STR "macos"
#define CIPPIE_HOST_ABI_ENUM cippie::Abi::darwin
#define CIPPIE_HOST_ABI_STR "darwin"
#else
#define CIPPIE_HOST_OS_ENUM cippie::Os::linux_
#define CIPPIE_HOST_OS_STR "linux"
#define CIPPIE_HOST_ABI_ENUM cippie::Abi::gnu
#define CIPPIE_HOST_ABI_STR "gnu"
#endif

namespace cippie
{
    namespace
    {
        Arch parseArch(const std::string& s)
        {
            if (s == "x86_64" || s == "x86-64" || s == "amd64") return Arch::x86_64;
            if (s == "aarch64" || s == "arm64") return Arch::aarch64;
            if (s == "arm" || s == "armv7" || s == "armhf") return Arch::arm;
            if (s == "i686" || s == "i386" || s == "x86") return Arch::i686;
            if (s == "riscv64") return Arch::riscv64;
            return Arch::unknown;
        }

        Os parseOs(const std::string& s)
        {
            if (s == "linux") return Os::linux_;
            if (s == "windows" || s == "win32" || s == "win") return Os::windows;
            if (s == "macos" || s == "darwin" || s == "osx") return Os::macos;
            if (s == "none" || s == "bare") return Os::none;
            return Os::unknown;
        }

        Abi parseAbi(const std::string& s)
        {
            if (s == "gnu" || s == "gnueabi" || s == "gnueabihf") return Abi::gnu;
            if (s == "musl") return Abi::musl;
            if (s == "msvc") return Abi::msvc;
            if (s == "mingw" || s == "mingw32" || s == "mingw64" || s == "gnu" ) return Abi::gnu;
            // mingw targets use gnu in our model
            if (s == "w64" || s == "pc") return Abi::gnu;
            if (s == "darwin") return Abi::darwin;
            if (s == "none" || s == "elf") return Abi::none;
            return Abi::unknown;
        }
    }

    std::string_view TargetTriple::archName(Arch a) noexcept
    {
        switch (a)
        {
            case Arch::x86_64:  return "x86_64";
            case Arch::aarch64: return "aarch64";
            case Arch::arm:     return "arm";
            case Arch::i686:    return "i686";
            case Arch::riscv64: return "riscv64";
            case Arch::unknown: return "unknown";
        }
        return "unknown";
    }

    std::string_view TargetTriple::osName(Os o) noexcept
    {
        switch (o)
        {
            case Os::linux_:  return "linux";
            case Os::windows: return "windows";
            case Os::macos:   return "macos";
            case Os::none:    return "none";
            case Os::unknown: return "unknown";
        }
        return "unknown";
    }

    std::string_view TargetTriple::abiName(Abi a) noexcept
    {
        switch (a)
        {
            case Abi::gnu:    return "gnu";
            case Abi::musl:   return "musl";
            case Abi::msvc:   return "msvc";
            case Abi::mingw:  return "mingw";
            case Abi::darwin: return "darwin";
            case Abi::none:   return "none";
            case Abi::unknown: return "unknown";
        }
        return "unknown";
    }

    std::string TargetTriple::toString() const
    {
        std::string result = std::string(archName(arch)) + "-" + std::string(osName(os));
        auto a = abiName(abi);
        if (a != "none" && a != "unknown")
        {
            result += "-";
            result += a;
        }
        return result;
    }

    bool TargetTriple::isNativeRunnable(const TargetTriple& host) const noexcept
    {
        return arch == host.arch && os == host.os;
    }

    TargetTriple TargetTriple::detectHost() noexcept
    {
        return TargetTriple{
            .arch = CIPPIE_HOST_ARCH_ENUM,
            .vendor = "pc",
            .os = CIPPIE_HOST_OS_ENUM,
            .abi = CIPPIE_HOST_ABI_ENUM
        };
    }

    Result<TargetTriple> TargetTriple::parse(const std::string& s)
    {
        if (s.empty())
        {
            return detectHost();
        }

        std::stringstream ss(s);
        std::string seg;
        std::vector<std::string> parts;
        while (std::getline(ss, seg, '-'))
        {
            if (!seg.empty()) parts.push_back(seg);
        }

        if (parts.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "empty target triple",
                .location = std::nullopt,
                .notes = {}
            });
        }

        TargetTriple t;
        t.arch = parseArch(parts[0]);
        if (t.arch == Arch::unknown)
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "unknown architecture '" + parts[0] + "' in triple '" + s + "'",
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (parts.size() == 1)
        {
            // Arch only — assume host OS/ABI
            auto host = detectHost();
            t.os = host.os;
            t.abi = host.abi;
            return t;
        }

        // Determine position of OS token: skip vendor token if needed
        // Heuristic: 2nd token is OS if it parses as a known OS; otherwise it's vendor
        size_t osIdx = 1;
        if (parts.size() >= 3 && parseOs(parts[1]) == Os::unknown)
        {
            // parts[1] is vendor
            t.vendor = parts[1];
            osIdx = 2;
        }

        if (osIdx < parts.size())
        {
            t.os = parseOs(parts[osIdx]);
            if (t.os == Os::unknown)
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "unknown OS '" + parts[osIdx] + "' in triple '" + s + "'",
                    .location = std::nullopt,
                    .notes = {}
                });
            }
        }

        // ABI is everything after OS
        if (osIdx + 1 < parts.size())
        {
            std::string abiStr = parts[osIdx + 1];
            for (size_t i = osIdx + 2; i < parts.size(); ++i)
            {
                abiStr += "-";
                abiStr += parts[i];
            }
            // Special: x86_64-linux-gnu is fine; x86_64-w64-mingw32 -> windows+gnu
            if (abiStr.find("mingw") != std::string::npos)
            {
                t.abi = Abi::gnu;
                if (t.os == Os::unknown) t.os = Os::windows; // mingw implies windows
            }
            else
            {
                t.abi = parseAbi(parts[osIdx + 1]);
            }
        }
        else
        {
            t.abi = Abi::none;
        }

        return t;
    }
}
