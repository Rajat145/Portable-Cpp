// Portable C++ (pcc) - stack-based virtual machine, Phase 1.
#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace pcc {

struct Limits {
    std::size_t   max_stack_depth = 1024;
    std::uint64_t max_steps       = 10'000'000;  // executed instructions
};

enum class Status {
    Ok,
    StackOverflow,
    StackUnderflow,
    DivideByZero,
    IntegerOverflow,   // INT64_MIN / -1
    InvalidOpcode,
    TruncatedOperand,
    PcOutOfRange,      // execution ran past the end without HALT
    StepLimitExceeded,
};

const char* status_name(Status s);

struct RunResult {
    Status        status = Status::Ok;
    std::size_t   pc     = 0;   // offset of the faulting (or last) instruction
    std::uint64_t steps  = 0;   // instructions executed
    std::string   message;
    bool ok() const { return status == Status::Ok; }
};

class Vm {
public:
    explicit Vm(Limits limits = Limits{}) : limits_(limits) {}

    // Runs `code` from offset 0. Never throws for bad bytecode; reports a Status.
    RunResult run(const std::vector<std::uint8_t>& code, std::ostream& out);

    // Operand stack as left after the last run (useful for tests/debugging).
    const std::vector<std::int64_t>& stack() const { return stack_; }

private:
    Limits                    limits_;
    std::vector<std::int64_t> stack_;
};

}  // namespace pcc
