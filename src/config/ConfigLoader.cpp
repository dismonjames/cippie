#include <cippie/config/ConfigLoader.hpp>
#include <cippie/config/Lexer.hpp>
#include <cippie/config/Parser.hpp>
#include <cippie/config/Validator.hpp>
#include <cippie/diagnostics/DiagnosticPrinter.hpp>

#include <fstream>
#include <sstream>

namespace cippie
{
    Result<Project> ConfigLoader::load(
        const std::filesystem::path& projectRoot
    ) const
    {
        const auto configurationFile = projectRoot / "Cippiefile";
        return loadFromFile(configurationFile);
    }

    Result<Project> ConfigLoader::loadFromFile(
        const std::filesystem::path& cippiefilePath
    ) const
    {
        if (!std::filesystem::is_regular_file(cippiefilePath))
        {
            return std::unexpected(Error{
                .code = ErrorCode::projectNotFound,
                .message = "Cippiefile does not exist: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::ifstream file(cippiefilePath, std::ios::binary);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "Failed to open Cippiefile: " + cippiefilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        const auto projectRoot = cippiefilePath.parent_path();

        Lexer lexer(content, cippiefilePath);
        auto tokens = lexer.tokenize();

        if (lexer.hasErrors())
        {
            const std::string formatted = DiagnosticPrinter::formatAll(
                lexer.diagnostics()
            );
            return std::unexpected(Error{
                .code = ErrorCode::invalidToken,
                .message = formatted,
                .location = std::nullopt,
                .notes = {}
            });
        }

        Parser parser(std::move(tokens), cippiefilePath);
        auto astProject = parser.parseProject();

        if (parser.hasErrors() || !astProject.has_value())
        {
            const std::string formatted = DiagnosticPrinter::formatAll(
                parser.diagnostics()
            );
            return std::unexpected(Error{
                .code = ErrorCode::parseFailed,
                .message = formatted,
                .location = std::nullopt,
                .notes = {}
            });
        }

        Validator validator;
        auto project = validator.validate(*astProject, projectRoot);

        if (validator.hasErrors() || !project.has_value())
        {
            const std::string formatted = DiagnosticPrinter::formatAll(
                validator.diagnostics()
            );
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = formatted,
                .location = std::nullopt,
                .notes = {}
            });
        }

        return *project;
    }
}
