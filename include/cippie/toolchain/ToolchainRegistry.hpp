#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/toolchain/Toolchain.hpp>

#include <filesystem>
#include <vector>

namespace cippie
{
    class ToolchainRegistry
    {
    public:
        ToolchainRegistry() = default;

        [[nodiscard]] static Result<ToolchainRegistry> load(
            const std::filesystem::path& configDir = getConfigDir()
        );

        [[nodiscard]] static std::filesystem::path getConfigDir();

        [[nodiscard]] Result<Toolchain> findForTarget(const TargetTriple& target) const;
        [[nodiscard]] Result<Toolchain> findByName(const std::string& name) const;
        [[nodiscard]] const std::vector<Toolchain>& toolchains() const noexcept { return m_toolchains; }

        void addToolchain(Toolchain t);

    private:
        std::vector<Toolchain> m_toolchains;
    };
}
