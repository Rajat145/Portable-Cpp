// Recursive-descent parser (Phase 2). Reports the first syntax error only.
//
//   program := statement* EOF
//   statement := "print" "(" expr ")" ";"
//   expr := term (("+" | "-") term)*          left-associative
//   term := unary (("*" | "/") unary)*        left-associative
//   unary := "-" unary | primary
//   primary := INTEGER | "(" expr ")"
#pragma once

#include <vector>

#include "pcc/ast.hpp"
#include "pcc/diagnostics.hpp"
#include "pcc/lexer.hpp"

namespace pcc {

struct ParseResult {
    Program     program;
    Diagnostics diagnostics;
    bool ok() const { return diagnostics.empty(); }
};

ParseResult parse(const std::vector<Token>& tokens);

}  // namespace pcc
