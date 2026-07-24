#pragma once

#include <string>

namespace cippie
{
    struct TargetTriple
    {
        std::string arch{"x86_64"};
        std::string vendor{"unknown"};
        std::string sys{"linux"};
        std::string abi{"gnu"};

        [[nodiscard]] std::string toString() const;
        [[nodiscard]] static TargetTriple detectHost();
        [[nodiscard]] static TargetTriple parse(const std::string& tripleString);
    };
}
