#pragma once

#include <cippie/build/BuildNode.hpp>
#include <cippie/core/Result.hpp>
#include <cippie/project/Project.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace cippie
{
    class BuildGraph
    {
    public:
        BuildGraph() = default;

        [[nodiscard]] static Result<BuildGraph> fromProject(const Project& project);

        [[nodiscard]] Result<std::vector<const Target*>> getExecutionPlan(
            const Target& rootTarget
        ) const;

        [[nodiscard]] const std::vector<BuildNode>& nodes() const noexcept { return m_nodes; }

    private:
        std::vector<BuildNode> m_nodes;
        std::unordered_map<std::string, const Target*> m_targetMap;
    };
}
