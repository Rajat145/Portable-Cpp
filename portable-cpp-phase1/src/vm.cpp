#include "pcc/vm.hpp"

#include <limits>
#include <utility>

#include "pcc/bytecode.hpp"

namespace pcc {

namespace {

// Well-defined uint64 -> int64 (modular) conversion, valid before C++20 too.
std::int64_t to_i64(std::uint64_t u) {
    if (u <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return static_cast<std::int64_t>(u);
    }
    return -static_cast<std::int64_t>(~u) - 1;
}

std::uint64_t to_u64(std::int64_t v) { return static_cast<std::uint64_t>(v); }

}  // namespace

const char* status_name(Status s) {
    switch (s) {
        case Status::Ok:                return "Ok";
        case Status::StackOverflow:     return "StackOverflow";
        case Status::StackUnderflow:    return "StackUnderflow";
        case Status::DivideByZero:      return "DivideByZero";
        case Status::IntegerOverflow:   return "IntegerOverflow";
        case Status::InvalidOpcode:     return "InvalidOpcode";
        case Status::TruncatedOperand:  return "TruncatedOperand";
        case Status::PcOutOfRange:      return "PcOutOfRange";
        case Status::StepLimitExceeded: return "StepLimitExceeded";
    }
    return "Unknown";
}

RunResult Vm::run(const std::vector<std::uint8_t>& code, std::ostream& out) {
    stack_.clear();
    std::size_t   pc    = 0;
    std::uint64_t steps = 0;

    auto fail = [&](Status s, std::size_t at, const char* msg) {
        RunResult r;
        r.status  = s;
        r.pc      = at;
        r.steps   = steps;
        r.message = msg;
        return r;
    };

    for (;;) {
        if (pc >= code.size()) {
            return fail(Status::PcOutOfRange, pc, "execution ran past end of code without HALT");
        }
        if (steps >= limits_.max_steps) {
            return fail(Status::StepLimitExceeded, pc, "instruction step limit exceeded");
        }
        const std::size_t  at  = pc;
        const std::uint8_t raw = code[pc++];
        if (!is_valid_opcode(raw)) {
            return fail(Status::InvalidOpcode, at, "invalid opcode");
        }
        ++steps;
        const Op op = static_cast<Op>(raw);

        // Decode operand (little-endian, byte by byte: host-endianness independent).
        std::int64_t      imm = 0;
        const std::size_t n   = operand_size(op);
        if (n != 0) {
            if (code.size() - pc < n) {
                return fail(Status::TruncatedOperand, at, "instruction operand is truncated");
            }
            std::uint64_t u = 0;
            for (std::size_t i = n; i-- > 0;) u = (u << 8) | code[pc + i];
            imm = to_i64(u);
            pc += n;
        }

        switch (op) {
            case Op::Halt: {
                RunResult r;
                r.status = Status::Ok;
                r.pc     = at;
                r.steps  = steps;
                return r;
            }

            case Op::PushI64:
                if (stack_.size() >= limits_.max_stack_depth) {
                    return fail(Status::StackOverflow, at, "operand stack limit exceeded");
                }
                stack_.push_back(imm);
                break;

            case Op::Pop:
                if (stack_.empty()) return fail(Status::StackUnderflow, at, "POP on empty stack");
                stack_.pop_back();
                break;

            case Op::Dup:
                if (stack_.empty()) return fail(Status::StackUnderflow, at, "DUP on empty stack");
                if (stack_.size() >= limits_.max_stack_depth) {
                    return fail(Status::StackOverflow, at, "operand stack limit exceeded");
                }
                stack_.push_back(stack_.back());
                break;

            case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: {
                if (stack_.size() < 2) {
                    return fail(Status::StackUnderflow, at, "binary operator needs two operands");
                }
                const std::int64_t b = stack_.back(); stack_.pop_back();
                const std::int64_t a = stack_.back(); stack_.pop_back();
                std::int64_t r = 0;
                if (op == Op::Add)      r = to_i64(to_u64(a) + to_u64(b));
                else if (op == Op::Sub) r = to_i64(to_u64(a) - to_u64(b));
                else if (op == Op::Mul) r = to_i64(to_u64(a) * to_u64(b));
                else {
                    if (b == 0) return fail(Status::DivideByZero, at, "division by zero");
                    if (a == std::numeric_limits<std::int64_t>::min() && b == -1) {
                        return fail(Status::IntegerOverflow, at, "INT64_MIN / -1 overflows");
                    }
                    r = a / b;  // C++11+: truncates toward zero
                }
                stack_.push_back(r);  // depth was reduced by 1, so within limit
                break;
            }

            case Op::Print:
                if (stack_.empty()) return fail(Status::StackUnderflow, at, "PRINT on empty stack");
                out << std::to_string(stack_.back()) << '\n';
                stack_.pop_back();
                break;
        }
    }
}

}  // namespace pcc
