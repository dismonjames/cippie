#include <cippie/project/TargetSelector.hpp>

#include <cassert>
#include <iostream>

int main()
{
    cippie::TargetSelector selector;

    // Test 1: Explicit target selection
    {
        cippie::Project project;
        project.name = "suite";
        project.targets.push_back(cippie::Target{.name = "client", .type = cippie::TargetType::executable});
        project.targets.push_back(cippie::Target{.name = "server", .type = cippie::TargetType::executable});

        auto res = selector.select(project, "server");
        assert(res.has_value());
        assert((*res)->name == "server");
    }

    // Test 2: Default target selection
    {
        cippie::Project project;
        project.name = "suite";
        project.defaultTarget = "client";
        project.targets.push_back(cippie::Target{.name = "client", .type = cippie::TargetType::executable});
        project.targets.push_back(cippie::Target{.name = "server", .type = cippie::TargetType::executable});

        auto res = selector.select(project, "");
        assert(res.has_value());
        assert((*res)->name == "client");
    }

    // Test 3: Auto-select single executable target
    {
        cippie::Project project;
        project.name = "single";
        project.targets.push_back(cippie::Target{.name = "core", .type = cippie::TargetType::staticLibrary});
        project.targets.push_back(cippie::Target{.name = "app", .type = cippie::TargetType::executable});

        auto res = selector.select(project, "");
        assert(res.has_value());
        assert((*res)->name == "app");
    }

    // Test 4: Ambiguous multiple executables report error
    {
        cippie::Project project;
        project.name = "multi";
        project.targets.push_back(cippie::Target{.name = "client", .type = cippie::TargetType::executable});
        project.targets.push_back(cippie::Target{.name = "server", .type = cippie::TargetType::executable});

        auto res = selector.select(project, "");
        assert(!res.has_value());
        assert(res.error().message.find("multiple runnable targets found") != std::string::npos);
        assert(res.error().message.find("client") != std::string::npos);
        assert(res.error().message.find("server") != std::string::npos);
    }

    std::cout << "All TargetSelector tests passed!\n";
    return 0;
}
