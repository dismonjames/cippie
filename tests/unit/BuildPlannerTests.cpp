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
    toolchain.target = cippie::TargetTriple{.arch = cippie::Arch::x86_64, .vendor = "pc", .os = cippie::Os::linux_, .abi = cippie::Abi::gnu};

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

    // Verify MSVC flag translation (-std=c++23 -> /std:c++latest)
    cippie::Toolchain msvcToolchain;
    msvcToolchain.name = "msvc";
    msvcToolchain.compilerFamily = cippie::CompilerFamily::msvc;
    msvcToolchain.cxxCompiler = "cl.exe";
    msvcToolchain.target = cippie::TargetTriple{.arch = cippie::Arch::x86_64, .vendor = "pc", .os = cippie::Os::windows, .abi = cippie::Abi::msvc};

    auto msvcPlan = planner.create(project, target, msvcToolchain, "debug");
    assert(msvcPlan.targetPlans.size() == 1);
    const auto& msvcTPlan = msvcPlan.targetPlans[0];
    assert(msvcTPlan.compileCommands.size() == 2);
    const auto& msvcOptions = msvcTPlan.compileCommands[0].options;
    bool foundStdLatest = false;
    bool foundInvalidStd = false;
    for (const auto& opt : msvcOptions)
    {
        if (opt == "/std:c++latest") foundStdLatest = true;
        if (opt == "-std=c++23") foundInvalidStd = true;
    }
    assert(foundStdLatest);
    assert(!foundInvalidStd);

    std::filesystem::remove_all(root);
    std::cout << "All BuildPlanner tests passed!\n";
    return 0;
}
