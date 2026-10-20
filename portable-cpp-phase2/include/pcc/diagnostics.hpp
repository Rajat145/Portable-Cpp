// Source locations and compiler diagnostics (Phase 2).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pcc {

// 1-based line; 1-based column counted in BYTES from the start of the line.
struct SourceLoc {
    std::uint32_t line   = 1;
    std::uint32_t column = 1;
};

struct Diagnostic {
    SourceLoc   loc;
    std::string message;
};

using Diagnostics = std::vector<Diagnostic>;

// "file:line:col: error: message" followed by the source line and a caret.
std::string format_diagnostic(const Diagnostic& d, const std::string& source,
                              const std::string& filename);

}  // namespace pcc
