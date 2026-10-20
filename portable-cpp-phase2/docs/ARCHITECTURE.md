# Architecture (through Phase 2)

    source -> lex() -> parse() -> Program (AST) -> generate_code() -> bytecode
              (Phase 2: all implemented; Sema arrives in Phase 3 with variables/types)
                                                                       |
                                                                       v
                                         Vm::run(code, std::ostream&) -> Status

    Libraries: pcc_core (bytecode + VM)  <-  pcc_frontend (compiler)  <-  pcc (CLI)

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
6. **Frontend depends on the core, never the reverse.** The VM can ship without the compiler.
7. **Errors are values.** The lexer collects diagnostics; the parser reports the first syntax
   error (internal exception, never escapes `parse()`). Recovery is deferred until the grammar
   is large enough for it to matter.
8. **Bounded recursion.** Parser depth and AST height are capped (512), so neither deeply nested
   input nor AST destruction/codegen can overflow the native stack.
9. **Unary minus lowers to `0 - x`.** Avoids a new opcode now; a NEG instruction may come later
   for tighter code, with identical semantics.
10. **No AST visitor / semantic pass yet.** One tagged `Expr` struct is enough for three node
    kinds; revisit when Phase 3 adds types and statements.
