#include <cippie/core/Version.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    // 1. Verify cippie::version() is exactly "0.1.6"
    std::string ver = std::string(cippie::version());
    assert(ver == "0.1.6");
    assert(ver.find("-dev") == std::string::npos);

    // 2. Read CMakeLists.txt and verify version matches
    std::ifstream cmakeFile("CMakeLists.txt");
    if (cmakeFile.is_open())
    {
        std::string line;
        bool found = false;
        while (std::getline(cmakeFile, line))
        {
            if (line.find("VERSION 0.1.6") != std::string::npos)
            {
                found = true;
                break;
            }
        }
        assert(found && "CMakeLists.txt must contain 'VERSION 0.1.6'");
    }

    std::cout << "Version consistency test passed: " << ver << "\n";
    return 0;
}
