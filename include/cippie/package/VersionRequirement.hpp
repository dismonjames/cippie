#pragma once

#include <cippie/package/SemanticVersion.hpp>
#include <cippie/core/Result.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace cippie
{
    enum class ComparatorOp
    {
        equal,
        greaterThan,
        greaterThanOrEqual,
        lessThan,
        lessThanOrEqual,
        wildcard
    };

    struct VersionComparator
    {
        ComparatorOp op{ComparatorOp::wildcard};
        SemanticVersion version;
    };

    class VersionRequirement
    {
    public:
        VersionRequirement() = default;

        [[nodiscard]] static Result<VersionRequirement> parse(std::string_view expr);

        [[nodiscard]] bool matches(const SemanticVersion& ver) const;
        [[nodiscard]] std::string toString() const;

        [[nodiscard]] const std::vector<VersionComparator>& comparators() const noexcept { return m_comparators; }

    private:
        std::vector<VersionComparator> m_comparators;
        std::string m_rawExpression;
    };
}
