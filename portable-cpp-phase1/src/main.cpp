// Phase 1 demo: runs a hand-written bytecode program computing 10 + 20.
#include <cstring>
#include <iostream>

#include "pcc/bytecode.hpp"
#include "pcc/vm.hpp"

int main(int argc, char** argv) {
    pcc::CodeBuilder b;
    b.push_i64(10).push_i64(20).emit(pcc::Op::Add).emit(pcc::Op::Print).emit(pcc::Op::Halt);

    if (argc > 1 && std::strcmp(argv[1], "--disasm") == 0) {
        std::cout << pcc::disassemble(b.bytes());
        return 0;
    }

    pcc::Vm vm;
    const pcc::RunResult r = vm.run(b.bytes(), std::cout);
    if (!r.ok()) {
        std::cerr << "runtime error: " << pcc::status_name(r.status) << " at pc=" << r.pc
                  << ": " << r.message << '\n';
        return 1;
    }
    return 0;
}
