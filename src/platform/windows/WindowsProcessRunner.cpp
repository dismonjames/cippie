#if defined(_WIN32) || defined(_WIN64)
#include <cippie/process/ProcessRunner.hpp>

#include <windows.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace cippie
{
    namespace
    {
        std::string buildCommandLine(const std::filesystem::path& executable, const std::vector<std::string>& args)
        {
            auto quote = [](const std::string& s) -> std::string
            {
                if (s.find(' ') != std::string::npos || s.find('\t') != std::string::npos)
                    return "\"" + s + "\"";
                return s;
            };

            std::string cmd = quote(executable.string());
            for (const auto& arg : args)
                cmd += " " + quote(arg);
            return cmd;
        }

    }

    ProcessResult ProcessRunner::run(const ProcessRequest& request) const
    {
        ProcessResult result;

        auto cmdLine = buildCommandLine(request.executable, request.arguments);
        std::string cmdLineCopy = cmdLine;

        HANDLE hStdoutRead = nullptr;
        HANDLE hStdoutWrite = nullptr;
        HANDLE hStderrRead = nullptr;
        HANDLE hStderrWrite = nullptr;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = nullptr;
        sa.bInheritHandle = true;

        if (request.captureOutput)
        {
            if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
            {
                result.exitCode = -1;
                result.exitedNormally = false;
                return result;
            }
            if (!CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0))
            {
                CloseHandle(hStdoutRead);
                CloseHandle(hStdoutWrite);
                result.exitCode = -1;
                result.exitedNormally = false;
                return result;
            }
            SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
            SetHandleInformation(hStderrRead, HANDLE_FLAG_INHERIT, 0);
        }

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = request.captureOutput ? hStdoutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = request.captureOutput ? hStderrWrite : GetStdHandle(STD_ERROR_HANDLE);

        // Convert command line to wide
        int wlen = MultiByteToWideChar(CP_UTF8, 0, cmdLineCopy.c_str(), -1, nullptr, 0);
        std::wstring wcmd(static_cast<std::size_t>(wlen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, cmdLineCopy.c_str(), -1, wcmd.data(), wlen);

        // Convert working directory to wide
        std::wstring wdir;
        if (!request.workingDirectory.empty())
        {
            auto dirStr = request.workingDirectory.string();
            int dwlen = MultiByteToWideChar(CP_UTF8, 0, dirStr.c_str(), -1, nullptr, 0);
            wdir.resize(static_cast<std::size_t>(dwlen));
            MultiByteToWideChar(CP_UTF8, 0, dirStr.c_str(), -1, wdir.data(), dwlen);
        }

        PROCESS_INFORMATION pi{};

        BOOL success = CreateProcessW(
            nullptr,
            wcmd.data(),
            nullptr,
            nullptr,
            true,
            CREATE_NO_WINDOW,
            nullptr,
            wdir.empty() ? nullptr : wdir.data(),
            &si,
            &pi
        );

        if (request.captureOutput)
        {
            CloseHandle(hStdoutWrite);
            CloseHandle(hStderrWrite);
        }

        if (!success)
        {
            if (request.captureOutput)
            {
                CloseHandle(hStdoutRead);
                CloseHandle(hStderrRead);
            }
            result.exitCode = -1;
            result.exitedNormally = false;
            return result;
        }

        CloseHandle(pi.hThread);

        // Read output if capturing
        if (request.captureOutput)
        {
            const DWORD bufSize = 4096;
            char buf[bufSize];
            DWORD bytesRead = 0;

            while (ReadFile(hStdoutRead, buf, bufSize, &bytesRead, nullptr) && bytesRead > 0)
                result.stdoutOutput.append(buf, bytesRead);

            bytesRead = 0;
            while (ReadFile(hStderrRead, buf, bufSize, &bytesRead, nullptr) && bytesRead > 0)
                result.stderrOutput.append(buf, bytesRead);

            CloseHandle(hStdoutRead);
            CloseHandle(hStderrRead);
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        result.exitCode = static_cast<int>(exitCode);
        result.exitedNormally = true;

        CloseHandle(pi.hProcess);

        return result;
    }
}
#endif
