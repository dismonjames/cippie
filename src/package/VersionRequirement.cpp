#include <cippie/package/VersionRequirement.hpp>

#include <sstream>

namespace cippie
{
    Result<VersionRequirement> VersionRequirement::parse(std::string_view expr)
    {
        VersionRequirement req;
        req.m_rawExpression = std::string(expr);

        // Trim leading and trailing whitespace
        while (!expr.empty() && std::isspace(static_cast<unsigned char>(expr.front()))) expr.remove_prefix(1);
        while (!expr.empty() && std::isspace(static_cast<unsigned char>(expr.back()))) expr.remove_suffix(1);

        if (expr.empty() || expr == "*")
        {
            req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::wildcard, .version = {}});
            return req;
        }

        // Split by space for conjunctions (e.g. ">=1.2.0 <2.0.0")
        size_t start = 0;
        while (start < expr.size())
        {
            while (start < expr.size() && std::isspace(static_cast<unsigned char>(expr[start]))) ++start;
            if (start >= expr.size()) break;

            size_t end = start;
            while (end < expr.size() && !std::isspace(static_cast<unsigned char>(expr[end]))) ++end;

            std::string_view token = expr.substr(start, end - start);
            start = end;

            if (token == "*")
            {
                req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::wildcard, .version = {}});
                continue;
            }

            ComparatorOp op = ComparatorOp::equal;
            std::string_view verStr = token;

            if (token.starts_with(">="))
            {
                op = ComparatorOp::greaterThanOrEqual;
                verStr = token.substr(2);
            }
            else if (token.starts_with(">"))
            {
                op = ComparatorOp::greaterThan;
                verStr = token.substr(1);
            }
            else if (token.starts_with("<="))
            {
                op = ComparatorOp::lessThanOrEqual;
                verStr = token.substr(2);
            }
            else if (token.starts_with("<"))
            {
                op = ComparatorOp::lessThan;
                verStr = token.substr(1);
            }
            else if (token.starts_with("="))
            {
                op = ComparatorOp::equal;
                verStr = token.substr(1);
            }
            else if (token.starts_with("^"))
            {
                std::string_view vStr = token.substr(1);
                auto vRes = SemanticVersion::parse(vStr);
                if (!vRes.has_value()) return std::unexpected(vRes.error());

                const auto& v = *vRes;
                // ^1.2.3  => >=1.2.3 <2.0.0
                // ^0.2.3  => >=0.2.3 <0.3.0
                // ^0.0.3  => >=0.0.3 <0.0.4
                SemanticVersion maxVer;
                if (v.major != 0)
                {
                    maxVer.major = v.major + 1;
                    maxVer.minor = 0;
                    maxVer.patch = 0;
                }
                else if (v.minor != 0)
                {
                    maxVer.major = 0;
                    maxVer.minor = v.minor + 1;
                    maxVer.patch = 0;
                }
                else
                {
                    maxVer.major = 0;
                    maxVer.minor = 0;
                    maxVer.patch = v.patch + 1;
                }

                req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::greaterThanOrEqual, .version = v});
                req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::lessThan, .version = maxVer});
                continue;
            }
            else if (token.starts_with("~"))
            {
                std::string_view vStr = token.substr(1);
                auto vRes = SemanticVersion::parse(vStr);
                if (!vRes.has_value()) return std::unexpected(vRes.error());

                const auto& v = *vRes;
                // ~1.2.3 => >=1.2.3 <1.3.0
                SemanticVersion maxVer;
                maxVer.major = v.major;
                maxVer.minor = v.minor + 1;
                maxVer.patch = 0;

                req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::greaterThanOrEqual, .version = v});
                req.m_comparators.push_back(VersionComparator{.op = ComparatorOp::lessThan, .version = maxVer});
                continue;
            }

            auto vRes = SemanticVersion::parse(verStr);
            if (!vRes.has_value()) return std::unexpected(vRes.error());

            req.m_comparators.push_back(VersionComparator{.op = op, .version = *vRes});
        }

        return req;
    }

    bool VersionRequirement::matches(const SemanticVersion& ver) const
    {
        if (m_comparators.empty()) return true;

        for (const auto& comp : m_comparators)
        {
            if (comp.op == ComparatorOp::wildcard) continue;

            // Prerelease matching policy:
            if (ver.isPrerelease())
            {
                bool sameCore = (ver.major == comp.version.major &&
                                 ver.minor == comp.version.minor &&
                                 ver.patch == comp.version.patch);
                if (!sameCore || !comp.version.isPrerelease())
                {
                    return false;
                }
            }

            switch (comp.op)
            {
                case ComparatorOp::equal:
                    if (ver != comp.version) return false;
                    break;
                case ComparatorOp::greaterThan:
                    if (ver <= comp.version) return false;
                    break;
                case ComparatorOp::greaterThanOrEqual:
                    if (ver < comp.version) return false;
                    break;
                case ComparatorOp::lessThan:
                    if (ver >= comp.version) return false;
                    break;
                case ComparatorOp::lessThanOrEqual:
                    if (ver > comp.version) return false;
                    break;
                case ComparatorOp::wildcard:
                    break;
            }
        }

        return true;
    }

    std::string VersionRequirement::toString() const
    {
        if (m_rawExpression.empty()) return "*";
        return m_rawExpression;
    }
}
