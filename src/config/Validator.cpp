#include <cippie/config/Validator.hpp>

#include <algorithm>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace cippie
{
    namespace
    {
        std::optional<std::string> getStringValue(const AstValue& val)
        {
            if (const auto* str = std::get_if<AstString>(&val))
            {
                return str->value;
            }
            if (const auto* id = std::get_if<AstIdentifier>(&val))
            {
                return id->name;
            }
            return std::nullopt;
        }

        std::optional<std::int64_t> getIntValue(const AstValue& val)
        {
            if (const auto* num = std::get_if<AstInteger>(&val))
            {
                return num->value;
            }
            return std::nullopt;
        }

        std::optional<bool> getBoolValue(const AstValue& val)
        {
            if (const auto* b = std::get_if<AstBoolean>(&val))
            {
                return b->value;
            }
            return std::nullopt;
        }

        bool getStringArray(
            const AstValue& val,
            std::vector<std::string>& outStrings
        )
        {
            const auto* arr = std::get_if<AstArray>(&val);
            if (!arr)
            {
                return false;
            }
            for (const auto& elem : arr->elements)
            {
                auto s = getStringValue(elem);
                if (s.has_value())
                {
                    outStrings.push_back(std::move(*s));
                }
                else if (const auto* call = std::get_if<AstCall>(&elem))
                {
                    if (call->name == "dependency" && !call->arguments.empty())
                    {
                        auto depName = getStringValue(call->arguments[0]);
                        if (depName.has_value())
                        {
                            outStrings.push_back(std::move(*depName));
                        }
                    }
                }
                else
                {
                    return false;
                }
            }
            return true;
        }
    }

    const std::vector<Diagnostic>& Validator::diagnostics() const noexcept
    {
        return m_diagnostics;
    }

    bool Validator::hasErrors() const noexcept
    {
        for (const auto& diag : m_diagnostics)
        {
            if (diag.severity == DiagnosticSeverity::error)
            {
                return true;
            }
        }
        return false;
    }

    void Validator::addDiagnostic(
        DiagnosticSeverity severity,
        std::string message,
        SourceLocation location
    )
    {
        m_diagnostics.push_back(Diagnostic{
            .severity = severity,
            .message = std::move(message),
            .location = location,
            .notes = {},
            .sourceLine = std::nullopt
        });
    }

    std::optional<Project> Validator::validate(
        const AstProjectDeclaration& astProject,
        const std::filesystem::path& projectRoot
    )
    {
        m_diagnostics.clear();

        if (astProject.name.empty())
        {
            addDiagnostic(
                DiagnosticSeverity::error,
                "project name must not be empty",
                astProject.location
            );
            return std::nullopt;
        }

        Project project;
        project.name = astProject.name;
        project.rootDirectory = projectRoot;
        project.configurationFile = projectRoot / "Cippiefile";

        validateProjectBody(astProject.body, project, projectRoot);

        // Target uniqueness check
        std::unordered_set<std::string> targetNames;
        for (const auto& target : project.targets)
        {
            if (targetNames.contains(target.name))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "duplicate target name '" + target.name + "'",
                    astProject.location
                );
            }
            else
            {
                targetNames.insert(target.name);
            }
        }

        // Default target validation
        if (project.defaultTarget.has_value())
        {
            if (!targetNames.contains(*project.defaultTarget))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "defaultTarget '" + *project.defaultTarget +
                        "' does not match any target in project",
                    astProject.location
                );
            }
        }

        // Dependencies uniqueness check
        std::unordered_set<std::string> depNames;
        for (const auto& dep : project.dependencies)
        {
            if (depNames.contains(dep.name))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "duplicate dependency name '" + dep.name + "'",
                    astProject.location
                );
            }
            else
            {
                depNames.insert(dep.name);
            }
        }

        // Links validation against targets or dependencies
        for (const auto& target : project.targets)
        {
            for (const auto& link : target.links)
            {
                if (!targetNames.contains(link) && !depNames.contains(link))
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "target '" + target.name + "' links unknown target or dependency '" +
                            link + "'",
                        astProject.location
                    );
                }
            }
        }

        // Target cycle detection
        detectCycles(project);

        if (hasErrors())
        {
            return std::nullopt;
        }

        return project;
    }

    bool Validator::validateProjectBody(
        const AstObject& body,
        Project& project,
        const std::filesystem::path& projectRoot
    )
    {
        std::unordered_set<std::string> seenKeys;

        for (const auto& stmt : body.statements)
        {
            if (const auto* assign = std::get_if<AstAssignment>(&stmt))
            {
                if (seenKeys.contains(assign->key))
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "duplicate assignment to '" + assign->key + "'",
                        assign->location
                    );
                    continue;
                }
                seenKeys.insert(assign->key);

                if (assign->key == "cpp")
                {
                    auto val = getIntValue(assign->value);
                    if (!val.has_value() || (*val != 17 && *val != 20 && *val != 23))
                    {
                        addDiagnostic(
                            DiagnosticSeverity::error,
                            "unsupported C++ standard (supported: 17, 20, 23)",
                            assign->location
                        );
                    }
                    else
                    {
                        project.cppStandard = static_cast<int>(*val);
                    }
                }
                else if (assign->key == "defaultTarget")
                {
                    auto val = getStringValue(assign->value);
                    if (!val.has_value())
                    {
                        addDiagnostic(
                            DiagnosticSeverity::error,
                            "defaultTarget must be a string",
                            assign->location
                        );
                    }
                    else
                    {
                        project.defaultTarget = std::move(*val);
                    }
                }
                else if (assign->key == "configurations")
                {
                    if (const auto* objPtr =
                            std::get_if<std::shared_ptr<AstObject>>(&assign->value))
                    {
                        if (*objPtr)
                        {
                            for (const auto& cfgStmt : (*objPtr)->statements)
                            {
                                if (const auto* cfgObj =
                                        std::get_if<AstNamedObjectDeclaration>(&cfgStmt))
                                {
                                    BuildConfiguration cfg;
                                    cfg.name = cfgObj->name;
                                    if (cfgObj->body)
                                    {
                                        validateConfiguration(
                                            cfgObj->name,
                                            *cfgObj->body,
                                            cfg
                                        );
                                    }
                                    project.configurations.push_back(std::move(cfg));
                                }
                            }
                        }
                    }
                }
                else if (assign->key == "dependencies")
                {
                    if (const auto* arr = std::get_if<AstArray>(&assign->value))
                    {
                        for (const auto& elem : arr->elements)
                        {
                            if (const auto* call = std::get_if<AstCall>(&elem))
                            {
                                if (call->name == "package" && call->arguments.size() >= 2)
                                {
                                    auto name = getStringValue(call->arguments[0]);
                                    auto ver = getStringValue(call->arguments[1]);
                                    if (name.has_value() && ver.has_value())
                                    {
                                        project.dependencies.push_back(Dependency{
                                            .name = *name,
                                            .sourceType = PackageSourceType::registry,
                                            .versionRequirement = *ver,
                                            .path = {},
                                            .url = {},
                                            .tag = {},
                                            .rev = {},
                                            .branch = {}
                                        });
                                    }
                                }
                                else if ((call->name == "pathPackage" || call->name == "path_package") && call->arguments.size() >= 2)
                                {
                                    auto name = getStringValue(call->arguments[0]);
                                    auto p = getStringValue(call->arguments[1]);
                                    if (name.has_value() && p.has_value())
                                    {
                                        project.dependencies.push_back(Dependency{
                                            .name = *name,
                                            .sourceType = PackageSourceType::path,
                                            .versionRequirement = "*",
                                            .path = *p,
                                            .url = {},
                                            .tag = {},
                                            .rev = {},
                                            .branch = {}
                                        });
                                    }
                                }
                                else if ((call->name == "gitPackage" || call->name == "git_package") && call->arguments.size() >= 2)
                                {
                                    auto name = getStringValue(call->arguments[0]);
                                    auto url = getStringValue(call->arguments[1]);
                                    std::string tag, rev, branch;

                                    for (size_t i = 2; i < call->arguments.size(); ++i)
                                    {
                                        if (const auto* assignPtr = std::get_if<std::shared_ptr<AstAssignment>>(&call->arguments[i]))
                                        {
                                            if (*assignPtr)
                                            {
                                                auto val = getStringValue((*assignPtr)->value);
                                                if (val.has_value())
                                                {
                                                    if ((*assignPtr)->key == "tag") tag = *val;
                                                    else if ((*assignPtr)->key == "rev") rev = *val;
                                                    else if ((*assignPtr)->key == "branch") branch = *val;
                                                }
                                            }
                                        }
                                    }

                                    if (!tag.empty() && !rev.empty())
                                    {
                                        addDiagnostic(
                                            DiagnosticSeverity::error,
                                            "gitPackage '" + name.value_or("") + "' cannot specify both tag and rev",
                                            call->location
                                        );
                                    }
                                    else if (name.has_value() && url.has_value())
                                    {
                                        project.dependencies.push_back(Dependency{
                                            .name = *name,
                                            .sourceType = PackageSourceType::git,
                                            .versionRequirement = "*",
                                            .path = {},
                                            .url = *url,
                                            .tag = tag,
                                            .rev = rev,
                                            .branch = branch
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "unknown project field '" + assign->key + "'",
                        assign->location
                    );
                }
            }
            else if (const auto* targetAst = std::get_if<AstTargetDeclaration>(&stmt))
            {
                validateTarget(*targetAst, project, projectRoot);
            }
            else if (const auto* namedObj = std::get_if<AstNamedObjectDeclaration>(&stmt))
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "unexpected block '" + namedObj->name + "' in project body",
                    namedObj->location
                );
            }
        }

        return true;
    }

    bool Validator::validateTarget(
        const AstTargetDeclaration& targetAst,
        Project& project,
        const std::filesystem::path& projectRoot
    )
    {
        Target target;
        target.name = targetAst.name;

        bool typeSet = false;

        if (targetAst.targetKind == "executable")
        {
            target.type = TargetType::executable;
            typeSet = true;
        }
        else if (targetAst.targetKind == "static_library")
        {
            target.type = TargetType::staticLibrary;
            typeSet = true;
        }
        else if (targetAst.targetKind == "shared_library")
        {
            target.type = TargetType::sharedLibrary;
            typeSet = true;
        }
        else if (targetAst.targetKind == "test")
        {
            target.type = TargetType::test;
            typeSet = true;
        }

        std::unordered_set<std::string> seenKeys;

        if (targetAst.body)
        {
            for (const auto& stmt : targetAst.body->statements)
            {
                if (const auto* assign = std::get_if<AstAssignment>(&stmt))
                {
                    if (seenKeys.contains(assign->key))
                    {
                        addDiagnostic(
                            DiagnosticSeverity::error,
                            "duplicate assignment to '" + assign->key + "' in target '" +
                                target.name + "'",
                            assign->location
                        );
                        continue;
                    }
                    seenKeys.insert(assign->key);

                    if (assign->key == "entry")
                    {
                        auto val = getStringValue(assign->value);
                        if (!val.has_value())
                        {
                            addDiagnostic(
                                DiagnosticSeverity::error,
                                "entry must be a string",
                                assign->location
                            );
                        }
                        else
                        {
                            target.entry = projectRoot / *val;
                        }
                    }
                    else if (assign->key == "type")
                    {
                        auto val = getStringValue(assign->value);
                        if (val == "static")
                        {
                            target.type = TargetType::staticLibrary;
                            typeSet = true;
                        }
                        else if (val == "shared")
                        {
                            target.type = TargetType::sharedLibrary;
                            typeSet = true;
                        }
                        else
                        {
                            addDiagnostic(
                                DiagnosticSeverity::error,
                                "invalid library type '" + val.value_or("") +
                                    "' (must be static or shared)",
                                assign->location
                            );
                        }
                    }
                    else if (assign->key == "sources")
                    {
                        std::vector<std::string> patterns;
                        if (!getStringArray(assign->value, patterns))
                        {
                            addDiagnostic(
                                DiagnosticSeverity::error,
                                "sources must be an array of strings",
                                assign->location
                            );
                        }
                        else
                        {
                            for (const auto& pat : patterns)
                            {
                                if (pat.find("..") != std::string::npos)
                                {
                                    addDiagnostic(
                                        DiagnosticSeverity::error,
                                        "source path escaping project root using '..' is rejected: " +
                                            pat,
                                        assign->location
                                    );
                                }
                            }
                            target.sourcePatterns = std::move(patterns);
                        }
                    }
                    else if (assign->key == "includes")
                    {
                        std::vector<std::string> incs;
                        if (!getStringArray(assign->value, incs))
                        {
                            addDiagnostic(
                                DiagnosticSeverity::error,
                                "includes must be an array of strings",
                                assign->location
                            );
                        }
                        else
                        {
                            for (auto&& inc : incs)
                            {
                                target.includeDirectories.push_back(projectRoot / inc);
                            }
                        }
                    }
                    else if (assign->key == "publicIncludes" || assign->key == "public_includes")
                    {
                        std::vector<std::string> incs;
                        if (!getStringArray(assign->value, incs))
                        {
                            addDiagnostic(
                                DiagnosticSeverity::error,
                                "publicIncludes must be an array of strings",
                                assign->location
                            );
                        }
                        else
                        {
                            for (auto&& inc : incs)
                            {
                                target.publicIncludeDirectories.push_back(projectRoot / inc);
                            }
                        }
                    }
                    else if (assign->key == "defines")
                    {
                        getStringArray(assign->value, target.compileDefinitions);
                    }
                    else if (assign->key == "compileOptions")
                    {
                        getStringArray(assign->value, target.compileOptions);
                    }
                    else if (assign->key == "linkOptions")
                    {
                        getStringArray(assign->value, target.linkOptions);
                    }
                    else if (assign->key == "links" || assign->key == "dependencies")
                    {
                        getStringArray(assign->value, target.links);
                    }
                    else
                    {
                        addDiagnostic(
                            DiagnosticSeverity::error,
                            "unknown target field '" + assign->key + "' in target '" +
                                target.name + "'",
                            assign->location
                        );
                    }
                }
            }
        }

        // Executable requirements
        if (targetAst.targetKind == "executable")
        {
            if (!target.entry.has_value())
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "executable target '" + target.name + "' must have an entry path",
                    targetAst.location
                );
            }
        }
        else if (targetAst.targetKind == "library")
        {
            if (!typeSet)
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "library target '" + target.name +
                        "' must explicitly specify type (static or shared)",
                    targetAst.location
                );
            }
            if (target.entry.has_value())
            {
                addDiagnostic(
                    DiagnosticSeverity::error,
                    "library target '" + target.name + "' must not have an entry path",
                    targetAst.location
                );
            }
        }

        project.targets.push_back(std::move(target));
        return true;
    }

    bool Validator::validateConfiguration(
        const std::string& /*name*/,
        const AstObject& body,
        BuildConfiguration& config
    )
    {
        std::unordered_set<std::string> seenKeys;

        for (const auto& stmt : body.statements)
        {
            if (const auto* assign = std::get_if<AstAssignment>(&stmt))
            {
                if (seenKeys.contains(assign->key))
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "duplicate assignment to '" + assign->key +
                            "' in configuration '" + config.name + "'",
                        assign->location
                    );
                    continue;
                }
                seenKeys.insert(assign->key);

                if (assign->key == "optimization")
                {
                    auto val = getIntValue(assign->value);
                    if (val.has_value())
                    {
                        config.optimization = static_cast<int>(*val);
                    }
                }
                else if (assign->key == "debugSymbols")
                {
                    auto val = getBoolValue(assign->value);
                    if (val.has_value())
                    {
                        config.debugSymbols = *val;
                    }
                }
                else if (assign->key == "warningsAsErrors")
                {
                    auto val = getBoolValue(assign->value);
                    if (val.has_value())
                    {
                        config.warningsAsErrors = *val;
                    }
                }
                else if (assign->key == "assertions")
                {
                    auto val = getBoolValue(assign->value);
                    if (val.has_value())
                    {
                        config.assertions = *val;
                    }
                }
                else if (assign->key == "lto")
                {
                    auto val = getBoolValue(assign->value);
                    if (val.has_value())
                    {
                        config.lto = *val;
                    }
                }
                else if (assign->key == "defines")
                {
                    getStringArray(assign->value, config.compileDefinitions);
                }
                else if (assign->key == "compileOptions")
                {
                    getStringArray(assign->value, config.compileOptions);
                }
                else if (assign->key == "linkOptions")
                {
                    getStringArray(assign->value, config.linkOptions);
                }
                else
                {
                    addDiagnostic(
                        DiagnosticSeverity::error,
                        "unknown field '" + assign->key + "' in configuration '" +
                            config.name + "'",
                        assign->location
                    );
                }
            }
        }
        return true;
    }

    void Validator::detectCycles(const Project& project)
    {
        std::unordered_map<std::string, std::vector<std::string>> graph;
        std::unordered_set<std::string> targetSet;

        for (const auto& target : project.targets)
        {
            targetSet.insert(target.name);
            for (const auto& link : target.links)
            {
                graph[target.name].push_back(link);
            }
        }

        std::unordered_map<std::string, int> state; // 0: unvisited, 1: visiting, 2: visited
        std::vector<std::string> path;

        auto dfs = [&](auto& self, const std::string& node) -> bool {
            state[node] = 1;
            path.push_back(node);

            auto it = graph.find(node);
            if (it != graph.end())
            {
                for (const auto& neighbor : it->second)
                {
                    if (targetSet.contains(neighbor))
                    {
                        if (state[neighbor] == 1)
                        {
                            std::string cycleMsg = "target dependency cycle detected: ";
                            auto cycleStart = std::find(path.begin(), path.end(), neighbor);
                            for (auto p = cycleStart; p != path.end(); ++p)
                            {
                                cycleMsg += *p + " -> ";
                            }
                            cycleMsg += neighbor;

                            SourceLocation loc;
                            addDiagnostic(DiagnosticSeverity::error, cycleMsg, loc);
                            return true;
                        }
                        if (state[neighbor] == 0)
                        {
                            if (self(self, neighbor))
                            {
                                return true;
                            }
                        }
                    }
                }
            }

            path.pop_back();
            state[node] = 2;
            return false;
        };

        for (const auto& target : project.targets)
        {
            if (state[target.name] == 0)
            {
                if (dfs(dfs, target.name))
                {
                    break;
                }
            }
        }
    }
}
