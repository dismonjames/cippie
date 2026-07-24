#pragma once

#include <cippie/core/Result.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cippie
{
    class FileHasher
    {
    public:
        FileHasher() = default;

        [[nodiscard]] static uint64_t fnv1a64(const void* data, size_t size, uint64_t seed = 0xcbf29ce484222325ULL) noexcept;

        [[nodiscard]] Result<std::string> hashFile(
            const std::filesystem::path& filePath,
            std::unordered_map<std::filesystem::path, std::string>* memo = nullptr
        ) const;

        [[nodiscard]] static std::string hashString(std::string_view str) noexcept;
    };
}
