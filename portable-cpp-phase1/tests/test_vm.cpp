// Dependency-free test harness for Phase 1 (a framework such as doctest/Catch2
// can be adopted later without changing the tests' meaning).
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>

#include "pcc/bytecode.hpp"
#include "pcc/vm.hpp"

using namespace pcc;
using Bytes = std::vector<std::uint8_t>;

static int g_failures = 0;
static int g_checks   = 0;

#define CHECK(cond)                                                                    \
    do {                                                                               \
        ++g_checks;                                                                    \
        if (!(cond)) {                                                                 \
            ++g_failures;                                                              \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
        }                                                                              \
    } while (0)

struct Out { RunResult r; std::string text; };

static Out run(const Bytes& code, Limits lim = Limits{}) {
    Vm vm(lim);
    std::ostringstream os;
    RunResult r = vm.run(code, os);
    return {r, os.str()};
}

static Bytes prog_binop(std::int64_t a, std::int64_t b, Op op) {
    CodeBuilder c;
    c.push_i64(a).push_i64(b).emit(op).emit(Op::Print).emit(Op::Halt);
    return c.bytes();
}

static void test_ten_plus_twenty() {
    CodeBuilder c;
    c.push_i64(10).push_i64(20).emit(Op::Add).emit(Op::Print).emit(Op::Halt);
    Out o = run(c.bytes());
    CHECK(o.r.ok());
    CHECK(o.text == "30\n");
    CHECK(o.r.steps == 5);
}

static void test_arithmetic() {
    CHECK(run(prog_binop(7, 5, Op::Sub)).text == "2\n");
    CHECK(run(prog_binop(6, 7, Op::Mul)).text == "42\n");
    CHECK(run(prog_binop(7, 2, Op::Div)).text == "3\n");
    CHECK(run(prog_binop(-7, 2, Op::Div)).text == "-3\n");  // truncates toward zero
    CHECK(run(prog_binop(7, -2, Op::Div)).text == "-3\n");
}

static void test_wraparound() {
    const std::int64_t mx = std::numeric_limits<std::int64_t>::max();
    const std::int64_t mn = std::numeric_limits<std::int64_t>::min();
    CHECK(run(prog_binop(mx, 1, Op::Add)).text == "-9223372036854775808\n");
    CHECK(run(prog_binop(mn, 1, Op::Sub)).text == "9223372036854775807\n");
    CHECK(run(prog_binop(mx, 2, Op::Mul)).text == "-2\n");
}

static void test_dup_pop() {
    CodeBuilder c;
    c.push_i64(4).emit(Op::Dup).emit(Op::Mul).emit(Op::Print)
     .push_i64(1).emit(Op::Pop).emit(Op::Halt);
    Out o = run(c.bytes());
    CHECK(o.r.ok());
    CHECK(o.text == "16\n");
}

static void test_divide_errors() {
    Out z = run(prog_binop(1, 0, Op::Div));
    CHECK(z.r.status == Status::DivideByZero);
    CHECK(z.text.empty());
    Out ov = run(prog_binop(std::numeric_limits<std::int64_t>::min(), -1, Op::Div));
    CHECK(ov.r.status == Status::IntegerOverflow);
}

static void test_stack_errors() {
    CHECK(run(Bytes{static_cast<std::uint8_t>(Op::Add)}).r.status == Status::StackUnderflow);
    CHECK(run(Bytes{static_cast<std::uint8_t>(Op::Pop)}).r.status == Status::StackUnderflow);
    CHECK(run(Bytes{static_cast<std::uint8_t>(Op::Print)}).r.status == Status::StackUnderflow);
    CHECK(run(Bytes{static_cast<std::uint8_t>(Op::Dup)}).r.status == Status::StackUnderflow);

    Limits lim; lim.max_stack_depth = 3;
    CodeBuilder c;
    c.push_i64(1).push_i64(2).push_i64(3).push_i64(4).emit(Op::Halt);
    CHECK(run(c.bytes(), lim).r.status == Status::StackOverflow);
    CodeBuilder ok;
    ok.push_i64(1).push_i64(2).push_i64(3).emit(Op::Halt);
    CHECK(run(ok.bytes(), lim).r.ok());  // exactly at the limit is allowed
    CodeBuilder d;
    d.push_i64(1).emit(Op::Dup).emit(Op::Dup).emit(Op::Dup).emit(Op::Halt);
    CHECK(run(d.bytes(), lim).r.status == Status::StackOverflow);
}

static void test_malformed_bytecode() {
    CHECK(run(Bytes{0xFF}).r.status == Status::InvalidOpcode);
    CHECK(run(Bytes{0x04}).r.status == Status::InvalidOpcode);
    CHECK(run(Bytes{}).r.status == Status::PcOutOfRange);
    CHECK(run(Bytes{0x01, 0x00, 0x00}).r.status == Status::TruncatedOperand);
    CHECK(run(Bytes{0x01}).r.status == Status::TruncatedOperand);
    CodeBuilder c;  // valid instructions but no HALT
    c.push_i64(1);
    CHECK(run(c.bytes()).r.status == Status::PcOutOfRange);
}

static void test_step_limit() {
    Limits lim; lim.max_steps = 2;
    Out o = run(prog_binop(1, 2, Op::Add), lim);
    CHECK(o.r.status == Status::StepLimitExceeded);
    CHECK(o.r.steps == 2);
}

static void test_encoding_is_little_endian() {
    CodeBuilder c;
    c.push_i64(0x0102030405060708LL);
    const Bytes expect{0x01, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
    CHECK(c.bytes() == expect);

    CodeBuilder m;
    m.push_i64(-1).emit(Op::Print).emit(Op::Halt);
    const Bytes minus_one{0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x00};
    CHECK(m.bytes() == minus_one);
    CHECK(run(minus_one).text == "-1\n");

    // Hand-written raw bytes decode identically regardless of host endianness.
    const Bytes raw{0x01, 0x2A, 0, 0, 0, 0, 0, 0, 0, 0x20, 0x00};
    CHECK(run(raw).text == "42\n");

    CodeBuilder mn;
    mn.push_i64(std::numeric_limits<std::int64_t>::min()).emit(Op::Print).emit(Op::Halt);
    CHECK(run(mn.bytes()).text == "-9223372036854775808\n");
}

static void test_disassembler() {
    CodeBuilder c;
    c.push_i64(10).push_i64(20).emit(Op::Add).emit(Op::Print).emit(Op::Halt);
    const std::string expect =
        "0000  PUSH_I64 10\n"
        "0009  PUSH_I64 20\n"
        "0018  ADD\n"
        "0019  PRINT\n"
        "0020  HALT\n";
    CHECK(disassemble(c.bytes()) == expect);
    CHECK(disassemble(Bytes{0xFF}).find("invalid opcode") != std::string::npos);
    CHECK(disassemble(Bytes{0x01, 0x00}).find("truncated") != std::string::npos);
}

int main() {
    test_ten_plus_twenty();
    test_arithmetic();
    test_wraparound();
    test_dup_pop();
    test_divide_errors();
    test_stack_errors();
    test_malformed_bytecode();
    test_step_limit();
    test_encoding_is_little_endian();
    test_disassembler();
    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
