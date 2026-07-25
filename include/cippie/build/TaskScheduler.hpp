#pragma once

#include <cippie/build/ExecutionNode.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cippie
{
    class TaskScheduler
    {
    public:
        TaskScheduler() = default;

        bool execute(
            std::vector<ExecutionNode>& nodes,
            size_t workerCount,
            const std::function<bool(ExecutionNode& node)>& executor
        );
    };
}
