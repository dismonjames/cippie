#include "helper.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << getGreeting();
    if (argc > 1)
    {
        std::cout << " Arg: " << argv[1];
    }
    std::cout << "\n";
    return 0;
}
