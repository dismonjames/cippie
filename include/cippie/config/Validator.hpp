#pragma once

#include <cippie/config/Ast.hpp>
#include <cippie/diagnostics/Diagnostic.hpp>
#include <cippie/project/Project.hpp>

#include <filesystem>
#include <vector>

namespace cippie
{
    class Validator
    {
    public:
        Validator() = default;

        [[nodiscard]] std::optional<Project> validate(
            const AstProjectDeclaration& astProject,
            const std::filesystem::path& projectRoot
        );

        [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;
        [[nodiscard]] bool hasErrors() const noexcept;

    private:
        void addDiagnostic(
            DiagnosticSeverity severity,
            std::string message,
            SourceLocation location
        );

        bool validateProjectBody(
            const AstObject& body,
            Project& project,
            const std::filesystem::path& projectRoot
        );

        bool validateTarget(
            const AstTargetDeclaration& targetAst,
            Project& project,
            const std::filesystem::path& projectRoot
        );

        bool validateConfiguration(
            const std::string& name,
            const AstObject& body,
            BuildConfiguration& config
        );

        void detectCycles(const Project& project);

        std::vector<Diagnostic> m_diagnostics;
    };
}
