#include <cippie/process/ProcessRunner.hpp>

#include <cassert>
#include <iostream>

int main()
{
    cippie::ProcessRunner runner;

    // Test 1: Substantial stdout & stderr output capture without deadlock
    {
        cippie::ProcessRequest req;
        req.executable = "/bin/sh";
        req.captureOutput = true;
        req.arguments = {
            "-c",
            "for i in $(seq 1 500); do echo \"stdout line $i\"; echo \"stderr line $i\" >&2; done"
        };

        auto res = runner.run(req);
        assert(res.exitCode == 0);
        assert(res.exitedNormally);
        assert(res.stdoutOutput.find("stdout line 500") != std::string::npos);
        assert(res.stderrOutput.find("stderr line 500") != std::string::npos);
    }

    std::cout << "All CapturedOutput tests passed!\n";
    return 0;
}
