#include <cippie/cache/FileHasher.hpp>

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace cippie
{
    uint64_t FileHasher::fnv1a64(const void* data, size_t size, uint64_t seed) noexcept
    {
        uint64_t hash = seed;
        const auto* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i)
        {
            hash ^= static_cast<uint64_t>(bytes[i]);
            hash *= 0x100000001b3ULL;
        }
        return hash;
    }

    std::string FileHasher::hashString(std::string_view str) noexcept
    {
        const uint64_t h = fnv1a64(str.data(), str.size());
        std::ostringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << h;
        return ss.str();
    }

    Result<std::string> FileHasher::hashFile(
        const std::filesystem::path& filePath,
        std::unordered_map<std::filesystem::path, std::string>* memo
    ) const
    {
        if (memo != nullptr)
        {
            auto it = memo->find(filePath);
            if (it != memo->end())
            {
                return it->second;
            }
        }

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open file for hashing: " + filePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        uint64_t hash = 0xcbf29ce484222325ULL;
        std::array<char, 65536> buffer;

        while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
        {
            const auto bytesRead = static_cast<size_t>(file.gcount());
            hash = fnv1a64(buffer.data(), bytesRead, hash);
        }

        std::ostringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << hash;
        std::string hexHash = ss.str();

        if (memo != nullptr)
        {
            (*memo)[filePath] = hexHash;
        }

        return hexHash;
    }
}
