#pragma once

#include <string>
#include <vector>

namespace cippie
{
    struct BuildConfiguration
    {
        std::string name;
        int optimization{0};
        bool debugSymbols{true};
        bool warningsAsErrors{false};
        bool assertions{true};
        bool lto{false};
        std::vector<std::string> compileDefinitions;
        std::vector<std::string> compileOptions;
        std::vector<std::string> linkOptions;
    };
}
