#include <cippie/toolchain/TargetTriple.hpp>

#include <sstream>
#include <vector>

#if defined(__x86_64__) || defined(_M_X64)
#define CIPPIE_HOST_ARCH "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define CIPPIE_HOST_ARCH "aarch64"
#else
#define CIPPIE_HOST_ARCH "x86_64"
#endif

#if defined(__linux__)
#define CIPPIE_HOST_SYS "linux"
#define CIPPIE_HOST_ABI "gnu"
#elif defined(_WIN32)
#define CIPPIE_HOST_SYS "windows"
#define CIPPIE_HOST_ABI "gnu"
#elif defined(__APPLE__)
#define CIPPIE_HOST_SYS "macos"
#define CIPPIE_HOST_ABI "darwin"
#else
#define CIPPIE_HOST_SYS "linux"
#define CIPPIE_HOST_ABI "gnu"
#endif

namespace cippie
{
    std::string TargetTriple::toString() const
    {
        if (abi.empty())
        {
            return arch + "-" + sys;
        }
        return arch + "-" + sys + "-" + abi;
    }

    TargetTriple TargetTriple::detectHost()
    {
        return TargetTriple{
            .arch = CIPPIE_HOST_ARCH,
            .vendor = "pc",
            .sys = CIPPIE_HOST_SYS,
            .abi = CIPPIE_HOST_ABI
        };
    }

    TargetTriple TargetTriple::parse(const std::string& tripleString)
    {
        if (tripleString.empty())
        {
            return detectHost();
        }

        std::stringstream ss(tripleString);
        std::string segment;
        std::vector<std::string> parts;
        while (std::getline(ss, segment, '-'))
        {
            parts.push_back(segment);
        }

        TargetTriple triple;
        if (!parts.empty())
        {
            triple.arch = parts[0];
        }
        if (parts.size() == 2)
        {
            triple.sys = parts[1];
            triple.abi = "";
        }
        else if (parts.size() == 3)
        {
            triple.sys = parts[1];
            triple.abi = parts[2];
        }
        else if (parts.size() >= 4)
        {
            triple.vendor = parts[1];
            triple.sys = parts[2];
            triple.abi = parts[3];
        }

        return triple;
    }
}
