// Code generation (Phase 2): AST -> bytecode.
#pragma once

#include <cstdint>
#include <vector>

#include "pcc/ast.hpp"

namespace pcc {

// Emits code for every statement and a final HALT. Cannot fail: the parser
// guarantees a well-formed, depth-bounded tree.
std::vector<std::uint8_t> generate_code(const Program& program);

}  // namespace pcc
