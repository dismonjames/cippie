#include <cippie/build/BuildGraph.hpp>

#include <cassert>
#include <iostream>

int main()
{
    // Test 1: Topological Ordering (client -> core -> util)
    {
        cippie::Project project;
        project.name = "suite";

        cippie::Target util{.name = "util", .type = cippie::TargetType::staticLibrary};
        cippie::Target core{.name = "core", .type = cippie::TargetType::staticLibrary, .links = {"util"}};
        cippie::Target client{.name = "client", .type = cippie::TargetType::executable, .links = {"core"}};

        project.targets = {client, core, util};

        auto graphRes = cippie::BuildGraph::fromProject(project);
        assert(graphRes.has_value());

        auto planRes = graphRes->getExecutionPlan(client);
        assert(planRes.has_value());

        const auto& plan = *planRes;
        assert(plan.size() == 3);
        assert(plan[0]->name == "util");
        assert(plan[1]->name == "core");
        assert(plan[2]->name == "client");
    }

    // Test 2: Cycle Detection (A -> B -> A)
    {
        cippie::Project project;
        project.name = "cycle";

        cippie::Target targetA{.name = "A", .type = cippie::TargetType::staticLibrary, .links = {"B"}};
        cippie::Target targetB{.name = "B", .type = cippie::TargetType::staticLibrary, .links = {"A"}};

        project.targets = {targetA, targetB};

        auto graphRes = cippie::BuildGraph::fromProject(project);
        assert(!graphRes.has_value());
        assert(graphRes.error().message.find("target dependency cycle detected") != std::string::npos);
    }

    std::cout << "All BuildGraph tests passed!\n";
    return 0;
}
