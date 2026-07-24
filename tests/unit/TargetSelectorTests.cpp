#include <cippie/project/TargetSelector.hpp>

#include <cassert>
#include <iostream>

int main()
{
    cippie::TargetSelector selector;

    // Test 1: Explicit build target selection
    {
        cippie::Project project;
        project.name = "suite";
        project.targets.push_back(cippie::Target{.name = "client", .type = cippie::TargetType::executable});
        project.targets.push_back(cippie::Target{.name = "server", .type = cippie::TargetType::executable});

        auto res = selector.selectForBuild(project, "server");
        assert(res.has_value());
        assert((*res)->name == "server");
    }

    // Test 2: Library run rejection
    {
        cippie::Project project;
        project.name = "suite";
        project.targets.push_back(cippie::Target{.name = "core", .type = cippie::TargetType::staticLibrary});

        auto res = selector.selectForRun(project, "core");
        assert(!res.has_value());
        assert(res.error().message.find("is a library and cannot be run") != std::string::npos);
    }

    // Test 3: Auto-select single executable for run
    {
        cippie::Project project;
        project.name = "single";
        project.targets.push_back(cippie::Target{.name = "core", .type = cippie::TargetType::staticLibrary});
        project.targets.push_back(cippie::Target{.name = "app", .type = cippie::TargetType::executable});

        auto res = selector.selectForRun(project, "");
        assert(res.has_value());
        assert((*res)->name == "app");
    }

    // Test 4: Ambiguous multiple executables for run
    {
        cippie::Project project;
        project.name = "multi";
        project.targets.push_back(cippie::Target{.name = "client", .type = cippie::TargetType::executable});
        project.targets.push_back(cippie::Target{.name = "server", .type = cippie::TargetType::executable});

        auto res = selector.selectForRun(project, "");
        assert(!res.has_value());
        assert(res.error().message.find("multiple runnable targets found") != std::string::npos);
    }

    // Test 5: Select test targets
    {
        cippie::Project project;
        project.name = "test_suite";
        project.targets.push_back(cippie::Target{.name = "unit_tests", .type = cippie::TargetType::test});
        project.targets.push_back(cippie::Target{.name = "int_tests", .type = cippie::TargetType::test});

        auto res = selector.selectForTest(project, "");
        assert(res.has_value());
        assert(res->size() == 2);
    }

    std::cout << "All TargetSelector tests passed!\n";
    return 0;
}
