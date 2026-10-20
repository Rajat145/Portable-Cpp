// Compiler facade (Phase 2): source text -> bytecode, or diagnostics.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "pcc/diagnostics.hpp"

namespace pcc {

struct CompileResult {
    std::vector<std::uint8_t> code;         // empty if there were errors
    Diagnostics               diagnostics;
    bool ok() const { return diagnostics.empty(); }
};

CompileResult compile_source(const std::string& source);

}  // namespace pcc
