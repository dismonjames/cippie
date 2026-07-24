#include <cippie/build/TaskScheduler.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    cippie::TaskScheduler scheduler;

    // Test 1: Diamond Graph Scheduler Execution
    //   A -> B
    //   A -> C
    //   B, C -> D
    {
        std::vector<cippie::ExecutionNode> nodes;
        nodes.push_back(cippie::ExecutionNode{.id = "A"});
        nodes.push_back(cippie::ExecutionNode{.id = "B", .dependencies = {"A"}});
        nodes.push_back(cippie::ExecutionNode{.id = "C", .dependencies = {"A"}});
        nodes.push_back(cippie::ExecutionNode{.id = "D", .dependencies = {"B", "C"}});

        bool success = scheduler.execute(nodes, 4, [](cippie::ExecutionNode& node) -> bool {
            return true;
        });

        assert(success);
        for (const auto& n : nodes)
        {
            assert(n.state == cippie::NodeState::succeeded);
        }
    }

    // Test 2: Node Failure Blocks Dependents but independent nodes finish
    //   A (fails) -> B (blocked)
    //   C (succeeds)
    {
        std::vector<cippie::ExecutionNode> nodes;
        nodes.push_back(cippie::ExecutionNode{.id = "A"});
        nodes.push_back(cippie::ExecutionNode{.id = "B", .dependencies = {"A"}});
        nodes.push_back(cippie::ExecutionNode{.id = "C"});

        bool success = scheduler.execute(nodes, 2, [](cippie::ExecutionNode& node) -> bool {
            if (node.id == "A") return false;
            return true;
        });

        assert(!success);
        assert(nodes[0].state == cippie::NodeState::failed);
        assert(nodes[1].state == cippie::NodeState::blocked);
        assert(nodes[2].state == cippie::NodeState::succeeded);
    }

    std::cout << "All TaskScheduler tests passed!\n";
    return 0;
}
