#include "pcc/bytecode.hpp"

#include <cstdio>

namespace pcc {

bool is_valid_opcode(std::uint8_t raw) {
    switch (raw) {
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x20:
            return true;
        default:
            return false;
    }
}

const char* op_name(Op op) {
    switch (op) {
        case Op::Halt:    return "HALT";
        case Op::PushI64: return "PUSH_I64";
        case Op::Pop:     return "POP";
        case Op::Dup:     return "DUP";
        case Op::Add:     return "ADD";
        case Op::Sub:     return "SUB";
        case Op::Mul:     return "MUL";
        case Op::Div:     return "DIV";
        case Op::Print:   return "PRINT";
    }
    return "?";
}

std::size_t operand_size(Op op) { return op == Op::PushI64 ? 8 : 0; }

CodeBuilder& CodeBuilder::emit(Op op) {
    bytes_.push_back(static_cast<std::uint8_t>(op));
    return *this;
}

CodeBuilder& CodeBuilder::push_i64(std::int64_t value) {
    emit(Op::PushI64);
    // int64 -> uint64 conversion is well defined (modular) in C++.
    const std::uint64_t u = static_cast<std::uint64_t>(value);
    for (int i = 0; i < 8; ++i) {
        bytes_.push_back(static_cast<std::uint8_t>((u >> (8 * i)) & 0xFFu));
    }
    return *this;
}

std::string disassemble(const std::vector<std::uint8_t>& code) {
    std::string out;
    std::size_t pc = 0;
    char buf[64];
    while (pc < code.size()) {
        std::snprintf(buf, sizeof buf, "%04zu  ", pc);
        out += buf;
        const std::uint8_t raw = code[pc];
        if (!is_valid_opcode(raw)) {
            std::snprintf(buf, sizeof buf, "<invalid opcode 0x%02X>\n", static_cast<unsigned>(raw));
            out += buf;
            ++pc;
            continue;
        }
        const Op op = static_cast<Op>(raw);
        out += op_name(op);
        const std::size_t n = operand_size(op);
        if (code.size() - pc - 1 < n) {
            out += " <truncated operand>\n";
            break;
        }
        if (n == 8) {
            std::uint64_t u = 0;
            for (int i = 7; i >= 0; --i) u = (u << 8) | code[pc + 1 + static_cast<std::size_t>(i)];
            const std::int64_t v = (u <= 0x7FFFFFFFFFFFFFFFull)
                                       ? static_cast<std::int64_t>(u)
                                       : -static_cast<std::int64_t>(~u) - 1;
            out += " " + std::to_string(v);
        }
        out += "\n";
        pc += 1 + n;
    }
    return out;
}

}  // namespace pcc
