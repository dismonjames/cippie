#include <cippie/project/ProjectGenerator.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Valid project names
    assert(cippie::ProjectGenerator::isValidProjectName("my_project"));
    assert(cippie::ProjectGenerator::isValidProjectName("my-project"));
    assert(cippie::ProjectGenerator::isValidProjectName("Project1"));
    assert(cippie::ProjectGenerator::isValidProjectName("a"));

    // Invalid project names
    assert(!cippie::ProjectGenerator::isValidProjectName(""));
    assert(!cippie::ProjectGenerator::isValidProjectName("123app"));
    assert(!cippie::ProjectGenerator::isValidProjectName("my project"));
    assert(!cippie::ProjectGenerator::isValidProjectName("my/project"));
    assert(!cippie::ProjectGenerator::isValidProjectName("my\\project"));
    assert(!cippie::ProjectGenerator::isValidProjectName("my:project"));

    std::cout << "All ProjectName tests passed!\n";
    return 0;
}
