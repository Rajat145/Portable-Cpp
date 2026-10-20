#include "pcc/diagnostics.hpp"

namespace pcc {

namespace {

std::string line_text(const std::string& src, std::uint32_t line) {
    std::size_t start = 0;
    for (std::uint32_t l = 1; l < line; ++l) {
        const std::size_t nl = src.find('\n', start);
        if (nl == std::string::npos) return "";
        start = nl + 1;
    }
    std::size_t end = src.find('\n', start);
    if (end == std::string::npos) end = src.size();
    std::string t = src.substr(start, end - start);
    if (!t.empty() && t.back() == '\r') t.pop_back();
    return t;
}

}  // namespace

std::string format_diagnostic(const Diagnostic& d, const std::string& source,
                              const std::string& filename) {
    std::string out = filename + ":" + std::to_string(d.loc.line) + ":" +
                      std::to_string(d.loc.column) + ": error: " + d.message + "\n";
    const std::string text = line_text(source, d.loc.line);
    out += "  " + text + "\n  ";
    for (std::size_t i = 0; i + 1 < d.loc.column; ++i) {
        out += (i < text.size() && text[i] == '\t') ? '\t' : ' ';  // keep tab alignment
    }
    out += "^\n";
    return out;
}

}  // namespace pcc
