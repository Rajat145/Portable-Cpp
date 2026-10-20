# Portable C++ (pcc)

An experimental toolchain: compile a C++-like language to a platform-independent
bytecode and run it on a small, portable virtual machine.

**Status: Phase 1 of 7.** Only a hand-written-bytecode stack VM exists. There is
no compiler, no file format, no C++ support yet. See `docs/CHECKLIST.md`.

## Build and test (Windows, Visual Studio 2022 / Build Tools, "x64 Native Tools Command Prompt")

    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure
    build\Release\pcc_phase1_demo.exe

Expected: the demo prints `30`; ctest reports `100% tests passed`.
(`ctest --test-dir` needs CMake >= 3.20; on older versions run `cd build && ctest -C Release`.)

MinGW-w64 / Linux / macOS (single-config generators):

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/pcc_phase1_demo

Disassemble the demo program: `pcc_phase1_demo --disasm`.

## Layout

    include/pcc/bytecode.hpp   opcodes, CodeBuilder, disassembler API
    include/pcc/vm.hpp         VM, limits, status codes
    src/bytecode.cpp, vm.cpp   implementation (portable core, no OS APIs)
    src/main.cpp               Phase 1 demo
    tests/test_vm.cpp          unit tests (no external dependencies)
    docs/                      architecture, bytecode spec, checklist
