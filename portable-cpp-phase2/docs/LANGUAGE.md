# Language definition (Phase 2 subset)

This is a **C++-like** language, not C++. `print(10 + 20);` at file scope is not
valid C++; `.cpp` is used only to match the planned `pcc compile hello.cpp` workflow.
Accepting real C++ is Phase 6 (Clang frontend).

## Grammar
    program   := statement* EOF
    statement := "print" "(" expr ")" ";"
    expr      := term (("+" | "-") term)*      left-associative
    term      := unary (("*" | "/") unary)*    left-associative
    unary     := "-" unary | primary
    primary   := INTEGER | "(" expr ")"

## Lexical rules
- ASCII only outside comments; whitespace = space, tab, CR, LF, FF, VT.
- Comments: `// ...` and `/* ... */` (non-nesting). An unterminated block comment is an error.
- Integer literals are decimal and must fit in a signed 64-bit integer
  (max 9223372036854775807). `007` is rejected (C++ would read octal). `10abc` is rejected.
  Consequence: `-9223372036854775808` cannot be written as a literal (same quirk as C++).
- A UTF-8 BOM is stripped by the `pcc` driver, not by the lexer.
- Columns in diagnostics are byte offsets, 1-based.

## Semantics (all defined; nothing is undefined behavior)
- The only type is a signed 64-bit integer.
- `+ - *` wrap modulo 2^64 (C++ signed overflow would be UB; we define it).
- `/` truncates toward zero. Division by zero and INT64_MIN / -1 are runtime errors.
- Unary `-x` is `0 - x`, so it wraps for INT64_MIN.
- Evaluation order: left operand before right operand.

## Implementation limits
- Expression nesting/height is limited to 512 levels (about 250 nested parentheses,
  or a chain of about 512 operators), enforced by the parser so that recursion
  in the compiler is bounded.
- Only the first syntax error is reported; all lexical errors (up to 50) are reported.
