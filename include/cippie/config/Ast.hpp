#pragma once

#include <cippie/diagnostics/SourceLocation.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace cippie
{
    struct AstString
    {
        std::string value;
        SourceLocation location;
    };

    struct AstInteger
    {
        std::int64_t value{0};
        SourceLocation location;
    };

    struct AstBoolean
    {
        bool value{false};
        SourceLocation location;
    };

    struct AstIdentifier
    {
        std::string name;
        SourceLocation location;
    };

    struct AstArray;
    struct AstCall;
    struct AstAssignment;
    struct AstObject;

    using AstValue = std::variant<
        AstString,
        AstInteger,
        AstBoolean,
        AstIdentifier,
        AstArray,
        AstCall,
        std::shared_ptr<AstAssignment>,
        std::shared_ptr<AstObject>
    >;

    struct AstArray
    {
        std::vector<AstValue> elements;
        SourceLocation location;
    };

    struct AstCall
    {
        std::string name;
        std::vector<AstValue> arguments;
        SourceLocation location;
    };

    struct AstAssignment
    {
        std::string key;
        AstValue value;
        SourceLocation location;
    };

    struct AstTargetDeclaration
    {
        std::string targetKind; // "executable", "library", "test"
        std::string name;
        std::shared_ptr<AstObject> body;
        SourceLocation location;
    };

    struct AstNamedObjectDeclaration
    {
        std::string name;
        std::shared_ptr<AstObject> body;
        SourceLocation location;
    };

    using AstStatement = std::variant<
        AstAssignment,
        AstTargetDeclaration,
        AstNamedObjectDeclaration
    >;

    struct AstObject
    {
        std::vector<AstStatement> statements;
        SourceLocation location;
    };

    struct AstProjectDeclaration
    {
        std::string name;
        AstObject body;
        SourceLocation location;
    };
}
