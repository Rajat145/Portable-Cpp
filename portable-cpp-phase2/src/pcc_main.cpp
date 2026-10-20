// pcc command-line driver (Phase 2).
//   pcc run    <source>   compile in memory and execute
//   pcc disasm <source>   compile and print the bytecode listing
// Exit codes: 0 ok, 1 compile error, 2 usage / I/O error, 3 runtime error.
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "pcc/bytecode.hpp"
#include "pcc/compiler.hpp"
#include "pcc/vm.hpp"

namespace {

void usage(std::ostream& os) {
    os << "usage:\n"
          "  pcc run    <source-file>\n"
          "  pcc disasm <source-file>\n";
}

bool read_file(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    if (out.size() >= 3 && out.compare(0, 3, "\xEF\xBB\xBF") == 0) out.erase(0, 3);  // UTF-8 BOM
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        usage(std::cout);
        return 0;
    }
    if (argc != 3) {
        usage(std::cerr);
        return 2;
    }
    const std::string cmd  = argv[1];
    const std::string path = argv[2];
    if (cmd != "run" && cmd != "disasm") {
        std::cerr << "pcc: unknown command '" << cmd << "'\n";
        usage(std::cerr);
        return 2;
    }

    std::string source;
    if (!read_file(path, source)) {
        std::cerr << "pcc: cannot read '" << path << "'\n";
        return 2;
    }

    const pcc::CompileResult cr = pcc::compile_source(source);
    if (!cr.ok()) {
        for (const pcc::Diagnostic& d : cr.diagnostics) {
            std::cerr << pcc::format_diagnostic(d, source, path);
        }
        return 1;
    }

    if (cmd == "disasm") {
        std::cout << pcc::disassemble(cr.code);
        return 0;
    }

    pcc::Vm vm;
    const pcc::RunResult rr = vm.run(cr.code, std::cout);
    std::cout.flush();
    if (!rr.ok()) {
        std::cerr << "runtime error: " << pcc::status_name(rr.status) << " at bytecode offset "
                  << rr.pc << ": " << rr.message << '\n';
        return 3;
    }
    return 0;
}
