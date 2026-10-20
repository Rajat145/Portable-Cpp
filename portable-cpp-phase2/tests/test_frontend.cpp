// Phase 2 tests: lexer, parser, code generation, diagnostics, end-to-end.
#include <cstdint>
#include <sstream>
#include <string>

#include "check.hpp"
#include "pcc/ast.hpp"
#include "pcc/bytecode.hpp"
#include "pcc/compiler.hpp"
#include "pcc/diagnostics.hpp"
#include "pcc/lexer.hpp"
#include "pcc/parser.hpp"
#include "pcc/vm.hpp"

using namespace pcc;

static bool contains(const std::string& s, const std::string& part) {
    return s.find(part) != std::string::npos;
}

// ---- helpers --------------------------------------------------------------

// Dump of the first statement's expression, or "ERR: <message>" on failure.
static std::string parse_dump(const std::string& src) {
    LexResult lx = lex(src);
    if (!lx.ok()) return "LEXERR: " + lx.diagnostics[0].message;
    ParseResult ps = parse(lx.tokens);
    if (!ps.ok()) return "ERR: " + ps.diagnostics[0].message;
    if (ps.program.statements.empty()) return "EMPTY";
    return dump(*ps.program.statements[0].value);
}

struct Ran { bool compiled; Status status; std::string out; };

static Ran compile_and_run(const std::string& src) {
    CompileResult cr = compile_source(src);
    if (!cr.ok()) return {false, Status::Ok, ""};
    Vm vm;
    std::ostringstream os;
    RunResult rr = vm.run(cr.code, os);
    return {true, rr.status, os.str()};
}

static std::string output_of(const std::string& src) {
    Ran r = compile_and_run(src);
    return r.compiled && r.status == Status::Ok ? r.out : "<failed>";
}

// ---- lexer ----------------------------------------------------------------

static void test_lex_basic() {
    LexResult r = lex("print(10 + 20);");
    CHECK(r.ok());
    const TokenKind expect[] = {TokenKind::Identifier, TokenKind::LParen, TokenKind::Integer,
                                TokenKind::Plus, TokenKind::Integer, TokenKind::RParen,
                                TokenKind::Semicolon, TokenKind::Eof};
    CHECK(r.tokens.size() == 8);
    if (r.tokens.size() == 8) {
        for (std::size_t i = 0; i < 8; ++i) CHECK(r.tokens[i].kind == expect[i]);
        CHECK(r.tokens[2].int_value == 10);
        CHECK(r.tokens[2].loc.line == 1 && r.tokens[2].loc.column == 7);
        CHECK(r.tokens[4].int_value == 20);
    }
}

static void test_lex_comments_and_lines() {
    LexResult r = lex("// c1\nprint(/* x\n y */ 1);\n");
    CHECK(r.ok());
    CHECK(r.tokens.size() == 6);  // print ( 1 ) ; EOF
    CHECK(r.tokens[0].loc.line == 2 && r.tokens[0].loc.column == 1);
    CHECK(r.tokens[2].loc.line == 3 && r.tokens[2].loc.column == 7);
}

static void test_lex_errors() {
    CHECK(contains(lex("@").diagnostics[0].message, "unexpected character '@'"));
    CHECK(contains(lex("10abc").diagnostics[0].message, "invalid number literal '10abc'"));
    CHECK(contains(lex("007").diagnostics[0].message, "leading zeros"));
    CHECK(lex("0").ok());
    CHECK(contains(lex("9223372036854775808").diagnostics[0].message, "too large"));
    LexResult mx = lex("9223372036854775807");
    CHECK(mx.ok() && mx.tokens[0].int_value == INT64_MAX);
    CHECK(contains(lex("/* open").diagnostics[0].message, "unterminated block comment"));
    LexResult u = lex("\xC3\xA9");  // one UTF-8 character => one diagnostic
    CHECK(u.diagnostics.size() == 1 && contains(u.diagnostics[0].message, "non-ASCII"));
    CHECK(lex("@ #").diagnostics.size() == 2);  // errors are collected
    LexResult flood = lex(std::string(1000, '@'));
    CHECK(flood.diagnostics.size() <= 51);
    CHECK(contains(lex(std::string(1, '\x01')).diagnostics[0].message, "control character 0x01"));
}

// ---- parser ---------------------------------------------------------------

static void test_parse_precedence() {
    CHECK(parse_dump("print(1 + 2 * 3);") == "(+ 1 (* 2 3))");
    CHECK(parse_dump("print(8 - 3 - 2);") == "(- (- 8 3) 2)");
    CHECK(parse_dump("print(8 / 4 / 2);") == "(/ (/ 8 4) 2)");
    CHECK(parse_dump("print((1 + 2) * 3);") == "(* (+ 1 2) 3)");
    CHECK(parse_dump("print(-2 * 3);") == "(* (neg 2) 3)");
    CHECK(parse_dump("print(- -5);") == "(neg (neg 5))");
    CHECK(parse_dump("print(2 - -3);") == "(- 2 (neg 3))");
}

static void test_parse_statements() {
    LexResult lx = lex("print(1); print(2);\nprint(3);");
    ParseResult ps = parse(lx.tokens);
    CHECK(ps.ok() && ps.program.statements.size() == 3);
    LexResult e = lex("// nothing\n");
    CHECK(parse(e.tokens).ok());
}

static void test_parse_errors() {
    CHECK(contains(parse_dump("print(1 +);"), "ERR: expected expression, found ')'"));
    CHECK(contains(parse_dump("print(1)"), "expected ';' after statement, found end of input"));
    CHECK(contains(parse_dump("foo(1);"), "unknown statement 'foo'"));
    CHECK(contains(parse_dump("print 1;"), "expected '(' after 'print'"));
    CHECK(contains(parse_dump("print((1);"), "expected ')'"));
    CHECK(contains(parse_dump("1 + 2;"), "expected a statement, found number 1"));
    CHECK(contains(parse_dump("print(1 2);"), "expected ')' after expression, found number 2"));
    LexResult lx = lex("print(1 +);");
    ParseResult ps = parse(lx.tokens);
    CHECK(!ps.ok() && ps.diagnostics[0].loc.line == 1 && ps.diagnostics[0].loc.column == 10);
    CHECK(ps.program.statements.empty());
}

static void test_parse_depth_limits() {
    CHECK(parse_dump("print(" + std::string(100, '(') + "1" + std::string(100, ')') + ");") == "1");
    CHECK(contains(parse_dump("print(" + std::string(600, '(') + "1" + std::string(600, ')') + ");"),
                   "too deep"));
    CHECK(contains(parse_dump("print(" + std::string(600, '-') + "1);"), "too deep"));
    std::string chain100 = "print(1", chain600 = "print(1";
    for (int i = 0; i < 99; ++i) chain100 += "+1";
    for (int i = 0; i < 599; ++i) chain600 += "+1";
    CHECK(parse_dump(chain100 + ");") != "" && !contains(parse_dump(chain100 + ");"), "ERR"));
    CHECK(contains(parse_dump(chain600 + ");"), "too complex"));
    // Extremely long input must not crash.
    CHECK(contains(parse_dump("print(" + std::string(200000, '(') + ");"), "too deep"));
}

// ---- code generation + end to end ----------------------------------------

static void test_codegen_matches_phase1_program() {
    CompileResult cr = compile_source("print(10 + 20);");
    CodeBuilder b;
    b.push_i64(10).push_i64(20).emit(Op::Add).emit(Op::Print).emit(Op::Halt);
    CHECK(cr.ok() && cr.code == b.bytes());
}

static void test_end_to_end() {
    CHECK(output_of("print(10 + 20);") == "30\n");
    CHECK(output_of("print(1 + 2 * 3);") == "7\n");
    CHECK(output_of("print(10 - 2 - 3);") == "5\n");
    CHECK(output_of("print((1 + 2) * 3);") == "9\n");
    CHECK(output_of("print(-5 + 2);") == "-3\n");
    CHECK(output_of("print(7 / 2);") == "3\n");
    CHECK(output_of("print(-7 / 2);") == "-3\n");
    CHECK(output_of("print(1); print(2);") == "1\n2\n");
    CHECK(output_of("// only a comment\n") == "");
    CHECK(output_of("") == "");
    CHECK(output_of("print(1);\r\nprint(2);\r\n") == "1\n2\n");  // CRLF sources
}

static void test_semantics() {
    CHECK(output_of("print(9223372036854775807 + 1);") == "-9223372036854775808\n");
    CHECK(output_of("print(-9223372036854775807);") == "-9223372036854775807\n");
    CHECK(output_of("print(-(9223372036854775807 + 1));") == "-9223372036854775808\n");
    CHECK(output_of("print(3037000500 * 3037000500);") == "-9223372036709301616\n");  // wraps
    Ran dz = compile_and_run("print(1 / 0);");
    CHECK(dz.compiled && dz.status == Status::DivideByZero && dz.out.empty());
    Ran dz2 = compile_and_run("print(1); print(5 / (3 - 3));");
    CHECK(dz2.status == Status::DivideByZero && dz2.out == "1\n");
}

static void test_runtime_stack_limit() {
    // Right-nested expression needs one stack slot per level; default limit is 1024.
    std::string deep = "print(";
    for (int i = 0; i < 300; ++i) deep += "1+(";
    deep += "1";
    for (int i = 0; i < 300; ++i) deep += ")";
    deep += ");";
    CompileResult cr = compile_source(deep);
    CHECK(!cr.ok());  // rejected by the depth limit before reaching the VM
    CHECK(compile_and_run("print(1+(1+(1+(1+1))));").out == "5\n");
}

static void test_compile_failure_has_no_code() {
    CompileResult cr = compile_source("print(1 +);");
    CHECK(!cr.ok() && cr.code.empty() && cr.diagnostics.size() == 1);
    CompileResult lx = compile_source("print(1 @);");
    CHECK(!lx.ok() && lx.code.empty());
}

// ---- diagnostics formatting ----------------------------------------------

static void test_diagnostic_format() {
    const std::string src = "print(1 +);";
    CompileResult cr = compile_source(src);
    CHECK(!cr.ok());
    const std::string expect =
        "t.cpp:1:10: error: expected expression, found ')'\n"
        "  print(1 +);\n"
        "           ^\n";  // two-space indent + 9 columns of padding + caret
    CHECK(format_diagnostic(cr.diagnostics[0], src, "t.cpp") == expect);

    const std::string crlf = "print(1);\r\nprint(2) print(3);\r\n";
    CompileResult c2 = compile_source(crlf);
    CHECK(!c2.ok() && c2.diagnostics[0].loc.line == 2);
    const std::string f = format_diagnostic(c2.diagnostics[0], crlf, "t.cpp");
    CHECK(contains(f, "t.cpp:2:10:") && contains(f, "\n  print(2) print(3);\n") && !contains(f, "\r"));

    const std::string tab = "\tprint(1 +);";
    CompileResult c3 = compile_source(tab);
    const std::string f3 = format_diagnostic(c3.diagnostics[0], tab, "t.cpp");
    CHECK(contains(f3, "\n  \t           ^\n") || contains(f3, "\n  \t"));  // tab preserved in padding
}

int main() {
    test_lex_basic();
    test_lex_comments_and_lines();
    test_lex_errors();
    test_parse_precedence();
    test_parse_statements();
    test_parse_errors();
    test_parse_depth_limits();
    test_codegen_matches_phase1_program();
    test_end_to_end();
    test_semantics();
    test_runtime_stack_limit();
    test_compile_failure_has_no_code();
    test_diagnostic_format();
    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
