#include <cippie/build/BuildPlanner.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-artifact-naming-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "src");
    std::ofstream(root / "src/dummy.cpp") << "void dummy() {}";

    cippie::Project project;
    project.name = "naming_suite";
    project.cppStandard = 23;
    project.rootDirectory = root;

    cippie::Target staticLib{.name = "foo", .type = cippie::TargetType::staticLibrary, .sourcePatterns = {"src/*.cpp"}};
    cippie::Target sharedLib{.name = "bar", .type = cippie::TargetType::sharedLibrary, .sourcePatterns = {"src/*.cpp"}};
    cippie::Target execTarget{.name = "app", .type = cippie::TargetType::executable, .sourcePatterns = {"src/*.cpp"}};

    project.targets = {staticLib, sharedLib, execTarget};

    cippie::Toolchain toolchain;
    toolchain.target = cippie::TargetTriple{.arch = cippie::Arch::x86_64, .vendor = "pc", .os = cippie::Os::linux_, .abi = cippie::Abi::gnu};

    cippie::BuildPlanner planner;

    // Test 1: Static library output name
    {
        auto plan = planner.create(project, staticLib, toolchain, "debug");
        assert(plan.targetPlans.size() == 1);
        assert(plan.targetPlans[0].artifactOutput.filename().string() == "libfoo.a");
    }

    // Test 2: Shared library output name
    {
        auto plan = planner.create(project, sharedLib, toolchain, "debug");
        assert(plan.targetPlans.size() == 1);
        assert(plan.targetPlans[0].artifactOutput.filename().string() == "libbar.so");
    }

    // Test 3: Executable output name
    {
        auto plan = planner.create(project, execTarget, toolchain, "debug");
        assert(plan.targetPlans.size() == 1);
        assert(plan.targetPlans[0].artifactOutput.filename().string() == "app");
    }

    std::filesystem::remove_all(root);
    std::cout << "All ArtifactNaming tests passed!\n";
    return 0;
}
