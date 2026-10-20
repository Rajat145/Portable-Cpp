# Bytecode specification (draft v0, Phase 1)

Not yet a file format: Phase 1 executes an in-memory byte buffer. A container
format (magic, version, sections, validation) arrives in Phase 4.

## Machine model
- Operand stack of signed 64-bit integers. Max depth is a VM limit (default 1024).
- Execution starts at offset 0. The program must end via `HALT`; running past
  the end of code is the error `PcOutOfRange`.
- A VM step limit (default 10,000,000 instructions) bounds execution.

## Encoding
- Instruction = 1 opcode byte + fixed operand bytes.
- Multi-byte operands are **little-endian**, two's complement, decoded
  byte-by-byte (never by casting memory), so results do not depend on host endianness.

## Instructions
| Opcode | Mnemonic  | Operand | Stack effect | Semantics |
|-------:|-----------|---------|--------------|-----------|
| 0x00 | HALT     | -       | -            | Stop successfully |
| 0x01 | PUSH_I64 | i64 (8B)| -> v         | Push constant |
| 0x02 | POP      | -       | v ->         | Discard top |
| 0x03 | DUP      | -       | v -> v v     | Duplicate top |
| 0x10 | ADD      | -       | a b -> a+b   | Wrapping modulo 2^64 |
| 0x11 | SUB      | -       | a b -> a-b   | Wrapping modulo 2^64 |
| 0x12 | MUL      | -       | a b -> a*b   | Wrapping modulo 2^64 |
| 0x13 | DIV      | -       | a b -> a/b   | Truncates toward zero; traps on b=0 (`DivideByZero`) and INT64_MIN/-1 (`IntegerOverflow`) |
| 0x20 | PRINT    | -       | v ->         | Writes decimal text of v and `\n` (no locale, no `\r`) |

Any other opcode byte is `InvalidOpcode`. A truncated operand is `TruncatedOperand`.

## Error model
All faults return a `Status` plus the offending instruction offset; the VM never
throws or exhibits undefined behavior on malformed bytecode.
