#include "pcc/compiler.hpp"

#include "pcc/codegen.hpp"
#include "pcc/lexer.hpp"
#include "pcc/parser.hpp"

namespace pcc {

CompileResult compile_source(const std::string& source) {
    CompileResult out;
    LexResult lx = lex(source);
    if (!lx.ok()) {
        out.diagnostics = std::move(lx.diagnostics);
        return out;
    }
    ParseResult ps = parse(lx.tokens);
    if (!ps.ok()) {
        out.diagnostics = std::move(ps.diagnostics);
        return out;
    }
    out.code = generate_code(ps.program);
    return out;
}

}  // namespace pcc
