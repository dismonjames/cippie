#pragma once

#include <cippie/package/SemanticVersion.hpp>
#include <cippie/project/Dependency.hpp>
#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct LockedPackage
    {
        std::string name;
        SemanticVersion version;
        PackageSourceType sourceType{PackageSourceType::registry};
        std::string sourceLocation;
        std::string commit;
        std::string integrity;
        std::vector<std::string> dependencies; // "name@version"

        auto operator<=>(const LockedPackage& other) const { return name <=> other.name; }
    };

    class LockFile
    {
    public:
        LockFile() = default;

        static constexpr uint32_t LOCK_SCHEMA_VERSION = 1;

        [[nodiscard]] static Result<LockFile> load(const std::filesystem::path& lockFilePath);
        [[nodiscard]] Result<void> save(const std::filesystem::path& lockFilePath) const;

        void addPackage(LockedPackage pkg);
        [[nodiscard]] const LockedPackage* findPackage(const std::string& name) const;
        [[nodiscard]] const std::vector<LockedPackage>& packages() const noexcept { return m_packages; }

    private:
        std::vector<LockedPackage> m_packages;
    };
}
