// Lexer (Phase 2): source text -> tokens. Collects all lexical errors.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "pcc/diagnostics.hpp"

namespace pcc {

enum class TokenKind {
    Integer, Identifier,
    Plus, Minus, Star, Slash, LParen, RParen, Semicolon,
    Eof,
};

struct Token {
    TokenKind     kind = TokenKind::Eof;
    SourceLoc     loc;
    std::string   text;           // the lexeme ("" for Eof)
    std::int64_t  int_value = 0;  // valid for Integer
};

struct LexResult {
    std::vector<Token> tokens;    // always ends with an Eof token
    Diagnostics        diagnostics;
    bool ok() const { return diagnostics.empty(); }
};

// Input must be ASCII outside comments. Strip any UTF-8 BOM before calling.
LexResult lex(const std::string& source);

}  // namespace pcc
