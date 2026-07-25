#pragma once

#include <cippie/cli/CommandLine.hpp>
#include <cippie/diagnostics/Logger.hpp>

namespace cippie
{
    class Application
    {
    public:
        Application() = default;

        [[nodiscard]] int run(int argc, char* argv[]);

    private:
        int dispatch(const CommandLine& commandLine);
        int buildProject(const CommandLine& commandLine);
        int runProject(const CommandLine& commandLine);
        int testProject(const CommandLine& commandLine);
        int cleanProject(const CommandLine& commandLine);
        int createNewProject(const CommandLine& commandLine);
        int restoreProject(const CommandLine& commandLine);
        int addPackage(const CommandLine& commandLine);
        int removePackage(const CommandLine& commandLine);
        int runDoctor(const CommandLine& commandLine);
        int runUpdate(const CommandLine& commandLine);

        void printHelp() const;
        void printVersion() const;

        Logger logger_;
    };
}
