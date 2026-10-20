// Portable C++ (pcc) - bytecode definitions, Phase 1.
// The normative description lives in docs/BYTECODE_SPEC.md.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace pcc {

enum class Op : std::uint8_t {
    Halt    = 0x00,  // stop execution successfully
    PushI64 = 0x01,  // operand: 8 bytes, little-endian two's complement
    Pop     = 0x02,
    Dup     = 0x03,
    Add     = 0x10,  // wrapping
    Sub     = 0x11,  // wrapping
    Mul     = 0x12,  // wrapping
    Div     = 0x13,  // truncates toward zero; traps on /0 and INT64_MIN / -1
    Print   = 0x20,  // pop value, write decimal + '\n' to the VM output
};

bool        is_valid_opcode(std::uint8_t raw);
const char* op_name(Op op);
std::size_t operand_size(Op op);  // number of operand bytes following the opcode

// Human-readable listing of a code buffer (never throws on malformed input).
std::string disassemble(const std::vector<std::uint8_t>& code);

// Small helper for hand-writing programs. Operands are encoded explicitly
// little-endian, independent of the host's endianness.
class CodeBuilder {
public:
    CodeBuilder& emit(Op op);
    CodeBuilder& push_i64(std::int64_t value);
    const std::vector<std::uint8_t>& bytes() const { return bytes_; }

private:
    std::vector<std::uint8_t> bytes_;
};

}  // namespace pcc
