# Architecture (Phase 1)

    [future] C++-like source -> Lexer -> Parser -> AST -> Sema -> Codegen
                                                                   |
                                                         bytecode (in memory)
                                                                   v
                                            Vm::run(code, std::ostream&)  -> Status

## Decisions
1. **Portable core vs. platform layer.** `pcc_core` uses only the C++17 standard
   library. OS-specific code (files, paths, console) will live behind an interface
   in a separate library; the VM takes an `std::ostream&` today for that reason.
2. **Stack VM over register VM.** Simplest to implement, validate and emit code for.
   Revisit only if profiling justifies it.
3. **All semantics specified, none inherited.** Overflow wraps, division truncates
   and traps, encoding is little-endian. Conversions use explicit well-defined helpers.
4. **VM never trusts bytecode.** Every decode and stack access is checked; faults are
   values (`Status`), not exceptions or UB.
5. **No test framework yet.** A 10-line macro avoids dependencies; adopt doctest/Catch2
   when tests outgrow it.
