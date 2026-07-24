#include <cippie/package/SemanticVersion.hpp>

#include <algorithm>
#include <charconv>
#include <sstream>

namespace cippie
{
    std::strong_ordering PrereleaseIdentifier::operator<=>(const PrereleaseIdentifier& other) const
    {
        if (isNumeric && other.isNumeric)
        {
            return numericValue <=> other.numericValue;
        }
        if (isNumeric && !other.isNumeric)
        {
            return std::strong_ordering::less; // Numeric < Alphanumeric
        }
        if (!isNumeric && other.isNumeric)
        {
            return std::strong_ordering::greater; // Alphanumeric > Numeric
        }
        return stringValue <=> other.stringValue;
    }

    bool SemanticVersion::operator==(const SemanticVersion& other) const
    {
        return (*this <=> other) == std::strong_ordering::equal;
    }

    std::strong_ordering SemanticVersion::operator<=>(const SemanticVersion& other) const
    {
        if (auto cmp = major <=> other.major; cmp != 0) return cmp;
        if (auto cmp = minor <=> other.minor; cmp != 0) return cmp;
        if (auto cmp = patch <=> other.patch; cmp != 0) return cmp;

        // Normal release > Prerelease
        if (prerelease.empty() && !other.prerelease.empty())
        {
            return std::strong_ordering::greater;
        }
        if (!prerelease.empty() && other.prerelease.empty())
        {
            return std::strong_ordering::less;
        }

        // Compare prerelease identifiers one by one
        const size_t minLen = std::min(prerelease.size(), other.prerelease.size());
        for (size_t i = 0; i < minLen; ++i)
        {
            if (auto cmp = prerelease[i] <=> other.prerelease[i]; cmp != 0)
            {
                return cmp;
            }
        }

        return prerelease.size() <=> other.prerelease.size();
    }

    namespace
    {
        bool isValidIdentifierChar(char c)
        {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') || c == '-';
        }

        Result<uint64_t> parseNumericComponent(std::string_view str)
        {
            if (str.empty())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "empty numeric version component",
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            if (str.size() > 1 && str[0] == '0')
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "invalid leading zero in numeric component: " + std::string(str),
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            uint64_t val = 0;
            auto res = std::from_chars(str.data(), str.data() + str.size(), val);
            if (res.ec != std::errc() || res.ptr != str.data() + str.size())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "invalid numeric version component: " + std::string(str),
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            return val;
        }
    }

    Result<SemanticVersion> SemanticVersion::parse(std::string_view str)
    {
        if (str.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "version string cannot be empty",
                .location = std::nullopt,
                .notes = {}
            });
        }

        SemanticVersion ver;

        // Split build metadata (+...)
        auto buildPos = str.find('+');
        std::string_view coreAndPre = str;
        if (buildPos != std::string_view::npos)
        {
            coreAndPre = str.substr(0, buildPos);
            std::string_view buildStr = str.substr(buildPos + 1);
            if (buildStr.empty())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "empty build metadata section",
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            size_t start = 0;
            while (start < buildStr.size())
            {
                auto dot = buildStr.find('.', start);
                std::string_view part = (dot == std::string_view::npos)
                                            ? buildStr.substr(start)
                                            : buildStr.substr(start, dot - start);

                if (part.empty())
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::validationFailed,
                        .message = "empty build metadata identifier",
                        .location = std::nullopt,
                        .notes = {}
                    });
                }

                for (char c : part)
                {
                    if (!isValidIdentifierChar(c))
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::validationFailed,
                            .message = "invalid character in build metadata: " + std::string(1, c),
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }
                }

                ver.buildMetadata.emplace_back(part);
                if (dot == std::string_view::npos) break;
                start = dot + 1;
            }
        }

        // Split prerelease (-...)
        auto prePos = coreAndPre.find('-');
        std::string_view coreStr = coreAndPre;
        if (prePos != std::string_view::npos)
        {
            coreStr = coreAndPre.substr(0, prePos);
            std::string_view preStr = coreAndPre.substr(prePos + 1);
            if (preStr.empty())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "empty prerelease section",
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            size_t start = 0;
            while (start < preStr.size())
            {
                auto dot = preStr.find('.', start);
                std::string_view part = (dot == std::string_view::npos)
                                            ? preStr.substr(start)
                                            : preStr.substr(start, dot - start);

                if (part.empty())
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::validationFailed,
                        .message = "empty prerelease identifier",
                        .location = std::nullopt,
                        .notes = {}
                    });
                }

                bool isNumeric = true;
                for (char c : part)
                {
                    if (!isValidIdentifierChar(c))
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::validationFailed,
                            .message = "invalid character in prerelease identifier: " + std::string(1, c),
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }
                    if (c < '0' || c > '9')
                    {
                        isNumeric = false;
                    }
                }

                PrereleaseIdentifier pid;
                if (isNumeric)
                {
                    if (part.size() > 1 && part[0] == '0')
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::validationFailed,
                            .message = "numeric prerelease identifier cannot have leading zero: " + std::string(part),
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }

                    uint64_t num = 0;
                    std::from_chars(part.data(), part.data() + part.size(), num);
                    pid.isNumeric = true;
                    pid.numericValue = num;
                }
                else
                {
                    pid.isNumeric = false;
                    pid.stringValue = std::string(part);
                }

                ver.prerelease.push_back(std::move(pid));
                if (dot == std::string_view::npos) break;
                start = dot + 1;
            }
        }

        // Parse major.minor.patch
        size_t firstDot = coreStr.find('.');
        size_t secondDot = (firstDot == std::string_view::npos) ? std::string_view::npos : coreStr.find('.', firstDot + 1);

        if (firstDot == std::string_view::npos || secondDot == std::string_view::npos || coreStr.find('.', secondDot + 1) != std::string_view::npos)
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "version must have major.minor.patch format: " + std::string(str),
                .location = std::nullopt,
                .notes = {}
            });
        }

        auto majRes = parseNumericComponent(coreStr.substr(0, firstDot));
        if (!majRes.has_value()) return std::unexpected(majRes.error());
        ver.major = *majRes;

        auto minRes = parseNumericComponent(coreStr.substr(firstDot + 1, secondDot - firstDot - 1));
        if (!minRes.has_value()) return std::unexpected(minRes.error());
        ver.minor = *minRes;

        auto patchRes = parseNumericComponent(coreStr.substr(secondDot + 1));
        if (!patchRes.has_value()) return std::unexpected(patchRes.error());
        ver.patch = *patchRes;

        return ver;
    }

    std::string SemanticVersion::toString() const
    {
        std::ostringstream ss;
        ss << major << "." << minor << "." << patch;

        if (!prerelease.empty())
        {
            ss << "-";
            for (size_t i = 0; i < prerelease.size(); ++i)
            {
                if (i > 0) ss << ".";
                if (prerelease[i].isNumeric)
                {
                    ss << prerelease[i].numericValue;
                }
                else
                {
                    ss << prerelease[i].stringValue;
                }
            }
        }

        if (!buildMetadata.empty())
        {
            ss << "+";
            for (size_t i = 0; i < buildMetadata.size(); ++i)
            {
                if (i > 0) ss << ".";
                ss << buildMetadata[i];
            }
        }

        return ss.str();
    }
}
