# Portable C++ (pcc)

An experimental toolchain: compile a C++-like language to a platform-independent
bytecode and run it on a small, portable virtual machine.

**Status: Phase 2 of 7.** A compiler for a tiny expression language
(`print(<integer expression>);`) targets the Phase 1 stack VM. No variables,
control flow, binary file format or real C++ yet. See `docs/CHECKLIST.md`.

## Build and test (Windows, Visual Studio 2022 / Build Tools, "x64 Native Tools Command Prompt")

    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

Expected: `100% tests passed, 0 tests failed out of 4`.
(`ctest --test-dir` needs CMake >= 3.20; otherwise `cd build && ctest -C Release --output-on-failure`.)

MinGW-w64 / Linux / macOS (single-config generators):

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure

## Use

    build\Release\pcc.exe run    examples\hello.cpp      -> 30
    build\Release\pcc.exe disasm examples\hello.cpp      -> bytecode listing
    build\Release\pcc.exe run    examples\bad.cpp        -> error with line, column and caret

Exit codes: 0 ok, 1 compile error, 2 usage/I-O error, 3 runtime error.
`pcc compile -o file.pbc` arrives with the binary format in Phase 4.

## Layout

    include/pcc/   bytecode.hpp vm.hpp | diagnostics.hpp lexer.hpp ast.hpp parser.hpp codegen.hpp compiler.hpp
    src/           core: bytecode.cpp vm.cpp | frontend: diagnostics lexer ast parser codegen compiler
                   drivers: pcc_main.cpp (pcc), main.cpp (Phase 1 demo)
    tests/         test_vm.cpp, test_frontend.cpp, check.hpp
    examples/      hello.cpp arithmetic.cpp bad.cpp
    docs/          ARCHITECTURE.md BYTECODE_SPEC.md LANGUAGE.md CHECKLIST.md
