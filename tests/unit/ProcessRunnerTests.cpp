#include <cippie/process/ProcessRunner.hpp>

#include <cassert>
#include <iostream>

int main()
{
    cippie::ProcessRunner runner;

    // Test 1: Exit code 0
    {
        cippie::ProcessRequest req;
        req.executable = "/bin/true";
        auto res = runner.run(req);
        assert(res.exitCode == 0);
        assert(res.exitedNormally);
    }

    // Test 2: Nonzero exit code
    {
        cippie::ProcessRequest req;
        req.executable = "/bin/sh";
        req.arguments = {"-c", "exit 42"};
        auto res = runner.run(req);
        assert(res.exitCode == 42);
        assert(res.exitedNormally);
    }

    // Test 3: Missing executable returns 127
    {
        cippie::ProcessRequest req;
        req.executable = "/bin/non_existent_binary_12345";
        auto res = runner.run(req);
        assert(res.exitCode == 127);
    }

    // Test 4: Argument forwarding with spaces
    {
        cippie::ProcessRequest req;
        req.executable = "/bin/echo";
        req.arguments = {"hello world", "foo bar"};
        auto res = runner.run(req);
        assert(res.exitCode == 0);
    }

    std::cout << "All ProcessRunner tests passed!\n";
    return 0;
}
