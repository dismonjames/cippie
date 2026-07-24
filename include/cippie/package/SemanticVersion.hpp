#pragma once

#include <cippie/core/Result.hpp>

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cippie
{
    struct PrereleaseIdentifier
    {
        bool isNumeric{false};
        uint64_t numericValue{0};
        std::string stringValue;

        std::strong_ordering operator<=>(const PrereleaseIdentifier& other) const;
        bool operator==(const PrereleaseIdentifier& other) const = default;
    };

    struct SemanticVersion
    {
        uint64_t major{0};
        uint64_t minor{0};
        uint64_t patch{0};
        std::vector<PrereleaseIdentifier> prerelease;
        std::vector<std::string> buildMetadata;

        [[nodiscard]] static Result<SemanticVersion> parse(std::string_view str);
        [[nodiscard]] std::string toString() const;

        [[nodiscard]] bool isPrerelease() const noexcept { return !prerelease.empty(); }

        std::strong_ordering operator<=>(const SemanticVersion& other) const;
        bool operator==(const SemanticVersion& other) const;
        bool operator<(const SemanticVersion& other) const { return (*this <=> other) == std::strong_ordering::less; }
        bool operator>(const SemanticVersion& other) const { return (*this <=> other) == std::strong_ordering::greater; }
        bool operator<=(const SemanticVersion& other) const { return (*this <=> other) != std::strong_ordering::greater; }
        bool operator>=(const SemanticVersion& other) const { return (*this <=> other) != std::strong_ordering::less; }
    };
}
