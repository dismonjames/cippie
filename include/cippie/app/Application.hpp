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

        [[nodiscard]] int buildProject(const CommandLine& commandLine);
        [[nodiscard]] int runProject(const CommandLine& commandLine);
        [[nodiscard]] int testProject(const CommandLine& commandLine);
        [[nodiscard]] int cleanProject(const CommandLine& commandLine);
        [[nodiscard]] int createNewProject(const CommandLine& commandLine);

        void printHelp() const;
        void printVersion() const;

        Logger logger_;
    };
}
