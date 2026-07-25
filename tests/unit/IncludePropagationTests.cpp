#include <cippie/build/BuildPlanner.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-include-prop-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "core/src");
    std::filesystem::create_directories(root / "core/include");
    std::filesystem::create_directories(root / "app/src");

    std::ofstream(root / "core/src/core.cpp") << "void core() {}";
    std::ofstream(root / "app/src/main.cpp") << "int main() {}";

    cippie::Project project;
    project.name = "prop_suite";
    project.cppStandard = 23;
    project.rootDirectory = root;

    cippie::Target core{
        .name = "core",
        .type = cippie::TargetType::staticLibrary,
        .sourcePatterns = {"core/src/**/*.cpp"},
        .publicIncludeDirectories = {root / "core/include"}
    };

    cippie::Target app{
        .name = "app",
        .type = cippie::TargetType::executable,
        .sourcePatterns = {"app/src/**/*.cpp"},
        .links = {"core"}
    };

    project.targets = {core, app};

    cippie::Toolchain toolchain;
    toolchain.target = cippie::TargetTriple{.arch = cippie::Arch::x86_64, .vendor = "pc", .os = cippie::Os::linux_, .abi = cippie::Abi::gnu};

    cippie::BuildPlanner planner;
    auto plan = planner.create(project, app, toolchain, "debug");

    assert(plan.targetPlans.size() == 2);

    // Find app's target plan
    const cippie::TargetBuildPlan* appPlan = nullptr;
    for (const auto& tp : plan.targetPlans)
    {
        if (tp.targetName == "app")
        {
            appPlan = &tp;
            break;
        }
    }

    assert(appPlan != nullptr);
    assert(!appPlan->compileCommands.empty());

    const auto& includes = appPlan->compileCommands[0].includeDirectories;
    bool foundPropagated = false;
    for (const auto& inc : includes)
    {
        if (inc == root / "core/include")
        {
            foundPropagated = true;
            break;
        }
    }

    assert(foundPropagated);

    std::filesystem::remove_all(root);
    std::cout << "All IncludePropagation tests passed!\n";
    return 0;
}
