#include <cippie/build/BuildGraph.hpp>

#include <algorithm>
#include <queue>
#include <sstream>
#include <unordered_set>

namespace cippie
{
    Result<BuildGraph> BuildGraph::fromProject(const Project& project)
    {
        BuildGraph graph;

        for (const auto& target : project.targets)
        {
            graph.m_targetMap[target.name] = &target;
        }

        std::unordered_set<std::string> targetNames;
        for (const auto& target : project.targets)
        {
            targetNames.insert(target.name);
        }

        for (const auto& target : project.targets)
        {
            BuildNode node;
            node.target = &target;

            for (const auto& link : target.links)
            {
                if (targetNames.contains(link))
                {
                    node.dependencies.push_back(link);
                }
            }

            graph.m_nodes.push_back(std::move(node));
        }

        // Cycle detection
        std::unordered_map<std::string, int> state; // 0: unvisited, 1: visiting, 2: visited
        std::vector<std::string> path;

        auto dfs = [&](auto& self, const std::string& name) -> std::optional<std::string> {
            state[name] = 1;
            path.push_back(name);

            auto it = graph.m_targetMap.find(name);
            if (it != graph.m_targetMap.end())
            {
                for (const auto& link : it->second->links)
                {
                    if (targetNames.contains(link))
                    {
                        if (state[link] == 1)
                        {
                            std::ostringstream ss;
                            ss << "target dependency cycle detected: ";
                            auto cycleStart = std::find(path.begin(), path.end(), link);
                            for (auto p = cycleStart; p != path.end(); ++p)
                            {
                                ss << *p << " -> ";
                            }
                            ss << link;
                            return ss.str();
                        }
                        if (state[link] == 0)
                        {
                            auto cycleErr = self(self, link);
                            if (cycleErr.has_value())
                            {
                                return cycleErr;
                            }
                        }
                    }
                }
            }

            path.pop_back();
            state[name] = 2;
            return std::nullopt;
        };

        for (const auto& target : project.targets)
        {
            if (state[target.name] == 0)
            {
                auto cycleErr = dfs(dfs, target.name);
                if (cycleErr.has_value())
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::dependencyCycle,
                        .message = *cycleErr,
                        .location = std::nullopt,
                        .notes = {}
                    });
                }
            }
        }

        return graph;
    }

    Result<std::vector<const Target*>> BuildGraph::getExecutionPlan(
        const Target& rootTarget
    ) const
    {
        std::vector<const Target*> plan;
        std::unordered_set<std::string> visited;

        auto dfs = [&](auto& self, const Target* target) -> void {
            visited.insert(target->name);

            for (const auto& link : target->links)
            {
                auto it = m_targetMap.find(link);
                if (it != m_targetMap.end())
                {
                    if (!visited.contains(it->second->name))
                    {
                        self(self, it->second);
                    }
                }
            }

            plan.push_back(target);
        };

        dfs(dfs, &rootTarget);
        return plan;
    }
}
