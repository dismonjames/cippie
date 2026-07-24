#pragma once

#include <cippie/cli/CommandLine.hpp>
#include <cippie/diagnostics/Logger.hpp>

namespace cippie
{
    class Application
    {
    public:
        [[nodiscard]] int run(int argc, char* argv[]);

    private:
        [[nodiscard]] int dispatch(const CommandLine& commandLine);

        [[nodiscard]] int buildProject(
            const CommandLine& commandLine,
            bool runAfterBuild
        );

        void printHelp() const;
        void printVersion() const;

        Logger logger_;
    };
}
