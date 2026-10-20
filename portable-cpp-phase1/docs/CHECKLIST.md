# Implementation checklist

- [x] Phase 1: opcodes, stack VM, hand-written 10+20 program, tests, CMake build
      (tested with g++ 13 on Linux with ASan/UBSan; MSVC and macOS: not yet verified)
- [ ] Phase 2: lexer, parser, AST, codegen for `print(10 + 20);`
- [ ] Phase 3: variables, types, control flow, functions, recursion
- [ ] Phase 4: binary format, validator, versioning, malformed-input tests
- [ ] Phase 5: Windows/Linux/macOS CI running one shared bytecode suite
- [ ] Phase 6: Clang AST investigation and subset translation
- [ ] Phase 7: VS Code extension, packaging, docs, releases
