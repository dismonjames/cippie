#pragma once

#include <cippie/config/TokenKind.hpp>
#include <cippie/diagnostics/SourceLocation.hpp>

#include <string_view>

namespace cippie
{
    struct Token
    {
        TokenKind kind{TokenKind::invalid};
        std::string_view lexeme;
        SourceLocation location;
    };
}
