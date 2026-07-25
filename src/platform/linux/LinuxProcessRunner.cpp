#if !defined(_WIN32) && !defined(_WIN64)
#include <cippie/process/ProcessRunner.hpp>

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace cippie
{
    ProcessResult ProcessRunner::run(const ProcessRequest& request) const
    {
        int stdoutPipe[2] = {-1, -1};
        int stderrPipe[2] = {-1, -1};

        if (request.captureOutput)
        {
            if (pipe(stdoutPipe) < 0 || pipe(stderrPipe) < 0)
            {
                if (stdoutPipe[0] != -1) { close(stdoutPipe[0]); close(stdoutPipe[1]); }
                if (stderrPipe[0] != -1) { close(stderrPipe[0]); close(stderrPipe[1]); }
                throw std::runtime_error("pipe failed: " + std::string(std::strerror(errno)));
            }
        }

        const pid_t child = fork();

        if (child < 0)
        {
            if (request.captureOutput)
            {
                close(stdoutPipe[0]); close(stdoutPipe[1]);
                close(stderrPipe[0]); close(stderrPipe[1]);
            }
            throw std::runtime_error(
                "fork failed: " + std::string(std::strerror(errno))
            );
        }

        if (child == 0)
        {
            if (request.captureOutput)
            {
                close(stdoutPipe[0]);
                close(stderrPipe[0]);
                dup2(stdoutPipe[1], STDOUT_FILENO);
                dup2(stderrPipe[1], STDERR_FILENO);
                close(stdoutPipe[1]);
                close(stderrPipe[1]);
            }

            if (
                !request.workingDirectory.empty() &&
                chdir(request.workingDirectory.c_str()) != 0
            )
            {
                _exit(126);
            }

            const std::string execPath = request.executable.string();

            std::vector<std::string> ownedArguments;
            ownedArguments.reserve(request.arguments.size() + 1);
            ownedArguments.push_back(execPath);

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

            execvp(execPath.c_str(), rawArguments.data());
            _exit(errno == ENOENT ? 127 : 126);
        }

        std::string stdoutOutput;
        std::string stderrOutput;

        if (request.captureOutput)
        {
            close(stdoutPipe[1]);
            close(stderrPipe[1]);

            // Set non-blocking read
            fcntl(stdoutPipe[0], F_SETFL, fcntl(stdoutPipe[0], F_GETFL) | O_NONBLOCK);
            fcntl(stderrPipe[0], F_SETFL, fcntl(stderrPipe[0], F_GETFL) | O_NONBLOCK);

            std::array<pollfd, 2> fds;
            fds[0] = pollfd{.fd = stdoutPipe[0], .events = POLLIN, .revents = 0};
            fds[1] = pollfd{.fd = stderrPipe[0], .events = POLLIN, .revents = 0};

            int openFds = 2;
            std::array<char, 4096> buf;

            while (openFds > 0)
            {
                int ret = poll(fds.data(), fds.size(), -1);
                if (ret < 0)
                {
                    if (errno == EINTR) continue;
                    break;
                }

                for (size_t i = 0; i < 2; ++i)
                {
                    if (fds[i].fd == -1) continue;

                    if (fds[i].revents & (POLLIN | POLLHUP | POLLERR))
                    {
                        for (;;)
                        {
                            ssize_t n = read(fds[i].fd, buf.data(), buf.size());
                            if (n > 0)
                            {
                                if (i == 0) stdoutOutput.append(buf.data(), static_cast<size_t>(n));
                                else stderrOutput.append(buf.data(), static_cast<size_t>(n));
                            }
                            else
                            {
                                if (n == 0 || (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)))
                                {
                                    if (n == 0 || (fds[i].revents & POLLHUP))
                                    {
                                        close(fds[i].fd);
                                        fds[i].fd = -1;
                                        openFds--;
                                    }
                                    break;
                                }
                                close(fds[i].fd);
                                fds[i].fd = -1;
                                openFds--;
                                break;
                            }
                        }
                    }
                }
            }

            if (fds[0].fd != -1) close(fds[0].fd);
            if (fds[1].fd != -1) close(fds[1].fd);
        }

        int status = 0;
        while (waitpid(child, &status, 0) < 0)
        {
            if (errno != EINTR)
            {
                throw std::runtime_error(
                    "waitpid failed: " + std::string(std::strerror(errno))
                );
            }
        }

        if (WIFEXITED(status))
        {
            return ProcessResult{
                .exitCode = WEXITSTATUS(status),
                .exitedNormally = true,
                .stdoutOutput = std::move(stdoutOutput),
                .stderrOutput = std::move(stderrOutput)
            };
        }

        if (WIFSIGNALED(status))
        {
            return ProcessResult{
                .exitCode = 128 + WTERMSIG(status),
                .exitedNormally = false,
                .stdoutOutput = std::move(stdoutOutput),
                .stderrOutput = std::move(stderrOutput)
            };
        }

        return ProcessResult{
            .exitCode = 1,
            .exitedNormally = false,
            .stdoutOutput = std::move(stdoutOutput),
            .stderrOutput = std::move(stderrOutput)
        };
    }
}
#endif
