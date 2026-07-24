#include <cippie/build/BuildPlanner.hpp>
#include <cippie/toolchain/Toolchain.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto root =
        std::filesystem::temp_directory_path() /
        "cippie-build-planner-test";

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "src/core");
    std::filesystem::create_directories(root / "apps/client");

    std::ofstream(root / "src/core/App.cpp") << "void coreApp() {}";
    std::ofstream(root / "apps/client/App.cpp") << "int main() {}";

    cippie::Project project;
    project.name = "suite";
    project.cppStandard = 23;
    project.rootDirectory = root;

    cippie::Target target;
    target.name = "client";
    target.type = cippie::TargetType::executable;
    target.sourcePatterns = {"src/core/**/*.cpp", "apps/client/**/*.cpp"};

    cippie::Toolchain toolchain;
    toolchain.target = cippie::TargetTriple{.arch = "x86_64", .vendor = "pc", .sys = "linux", .abi = "gnu"};

    cippie::BuildPlanner planner;
    auto plan = planner.create(project, target, toolchain, "debug");

    assert(plan.targetPlans.size() == 1);
    const auto& tPlan = plan.targetPlans[0];
    assert(tPlan.compileCommands.size() == 2);

    const auto obj0 = tPlan.compileCommands[0].object.string();
    const auto obj1 = tPlan.compileCommands[1].object.string();

    assert(obj0 != obj1);
    assert(obj0.find("apps/client/App.cpp.o") != std::string::npos);
    assert(obj1.find("src/core/App.cpp.o") != std::string::npos);

    // Verify bin path
    assert(tPlan.linkCommand.has_value());
    const auto binPath = tPlan.linkCommand->output.string();
    assert(binPath.find(".cippie/build/x86_64-linux-gnu/debug/client/bin/client") != std::string::npos);

    std::filesystem::remove_all(root);
    std::cout << "All BuildPlanner tests passed!\n";
    return 0;
}
