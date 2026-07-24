#pragma once

#include <cippie/core/Result.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace cippie
{
    class SHA256
    {
    public:
        SHA256();

        void update(const void* data, size_t size);
        std::string finalHex();

        [[nodiscard]] static Result<std::string> hashFile(const std::filesystem::path& filePath);
        [[nodiscard]] static std::string hashString(std::string_view str);

    private:
        void transform(const uint8_t data[64]);

        uint32_t m_state[8];
        uint64_t m_count{0};
        uint8_t m_buffer[64];
    };
}
