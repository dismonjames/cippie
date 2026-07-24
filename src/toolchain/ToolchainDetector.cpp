#include <cippie/toolchain/ToolchainDetector.hpp>

#include <cstdlib>

namespace cippie
{
    Toolchain ToolchainDetector::detect() const
    {
        Toolchain toolchain;

        if (const char* compiler = std::getenv("CXX"))
        {
            toolchain.cxxCompiler = compiler;
            toolchain.linker = compiler;
        }

        return toolchain;
    }
}
