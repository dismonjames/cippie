#include <cippie/process/Process.hpp>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace cippie
{
    ProcessResult Process::run(const ProcessRequest& request) const
    {
        const pid_t child = fork();

        if (child < 0)
        {
            throw std::runtime_error(
                "fork failed: " + std::string(std::strerror(errno))
            );
        }

        if (child == 0)
        {
            if (
                !request.workingDirectory.empty() &&
                chdir(request.workingDirectory.c_str()) != 0
            )
            {
                _exit(126);
            }

            std::vector<std::string> ownedArguments;
            ownedArguments.reserve(request.arguments.size() + 1);
            ownedArguments.push_back(request.executable);

            for (const auto& argument : request.arguments)
            {
                ownedArguments.push_back(argument);
            }

            std::vector<char*> rawArguments;
            rawArguments.reserve(ownedArguments.size() + 1);

            for (auto& argument : ownedArguments)
            {
                rawArguments.push_back(argument.data());
            }

            rawArguments.push_back(nullptr);

            execvp(request.executable.c_str(), rawArguments.data());
            _exit(errno == ENOENT ? 127 : 126);
        }

        int status = 0;

        while (waitpid(child, &status, 0) < 0)
        {
            if (errno != EINTR)
            {
                throw std::runtime_error(
                    "waitpid failed: " +
                    std::string(std::strerror(errno))
                );
            }
        }

        if (WIFEXITED(status))
        {
            return {
                .exitCode = WEXITSTATUS(status),
                .exitedNormally = true
            };
        }

        if (WIFSIGNALED(status))
        {
            return {
                .exitCode = 128 + WTERMSIG(status),
                .exitedNormally = false
            };
        }

        return {
            .exitCode = 1,
            .exitedNormally = false
        };
    }
}
