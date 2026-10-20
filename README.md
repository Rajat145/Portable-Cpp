# Portable C++

**An experimental project exploring cross-platform execution of C++ programs using a custom bytecode and virtual machine.**

Portable C++ aims to make C++ programs easier to execute across different operating systems by exploring a compiler pipeline that translates supported C++ constructs into platform-independent bytecode, which can then be executed by a compatible virtual machine.

The long-term goal is to build a developer-friendly toolchain that reduces dependence on platform-specific executable binaries for supported programs.

> **Project status:** Experimental and under active development. The current phases are prototypes, not a production-ready replacement for a standard C++ compiler.

## Vision

C++ programs are commonly compiled into native machine code for a particular target platform and architecture. Portable C++ explores an alternative execution pipeline:

```text
C++ Source Code
      |
      v
Compiler Frontend
      |
      v
Intermediate Representation
      |
      v
Custom Bytecode
      |
      v
Portable C++ Virtual Machine
      |
      v
Program Execution
```

The intention is to allow the same compatible bytecode to execute on different operating systems that provide a compatible runtime.

**Important:** Bytecode alone does not guarantee portability. The virtual machine, runtime, system interfaces, and supported language features must also be compatible across platforms.

## Project Structure

```text
Portable-C++/
├── portable-cpp-phase1/
│   └── portable-cpp/
│       ├── docs/
│       ├── include/
│       ├── src/
│       ├── tests/
│       ├── CMakeLists.txt
│       └── README.md
│
├── portable-cpp-phase2/
│   └── portable-cpp/
│       ├── docs/
│       ├── examples/
│       ├── include/
│       ├── src/
│       ├── tests/
│       ├── CMakeLists.txt
│       └── README.md
│
└── README.md
```

Each phase contains its own implementation, documentation, and build instructions. This repository-level README describes the overall project, while individual phase READMEs document the implementation details of their respective milestones.

## Development Roadmap

### Phase 1 — Bytecode Virtual Machine

**Goal:** Establish the foundation for executing custom bytecode.

- Implement the initial virtual machine.
- Define basic bytecode instructions.
- Execute manually constructed bytecode.
- Add tests for VM behavior.

### Phase 2 — Basic Compiler

**Goal:** Introduce a compiler frontend that generates bytecode for a small supported language subset.

- Lexical analysis and tokenization.
- Parsing and abstract syntax tree construction.
- Basic expression handling and code generation.
- Execution and bytecode disassembly.
- Compiler diagnostics and tests.

The input syntax supported by this prototype may be a restricted language subset rather than valid, general-purpose C++.

### Phase 3 — Language Features

**Planned work:**

- Variables and data types.
- Conditional statements and loops.
- Functions and recursion.
- Comparisons and logical operations.
- Additional tests and diagnostics.

### Phase 4 — Persistent Bytecode Format

**Planned work:**

- Save and load compiled bytecode files.
- Define a versioned bytecode format.
- Validate bytecode before execution.
- Improve error handling and compatibility.

### Phase 5 — Cross-Platform Runtime

**Planned work:**

- Test the runtime on Windows, Linux, and macOS.
- Identify and eliminate platform-dependent assumptions.
- Establish repeatable builds and automated tests.
- Verify that compatible bytecode behaves consistently across platforms.

### Phase 6 — Clang/LLVM Integration

**Planned work:**

- Explore using Clang to parse actual C++ source code.
- Translate a clearly defined subset of supported C++ constructs into the project's bytecode.
- Handle unsupported language constructs with useful diagnostics.
- Evaluate integration with the C++ ecosystem and runtime libraries.

Full C++ compatibility is not assumed. Features such as templates, exceptions, pointers, the standard library, and native library interoperability may require substantial additional work.

### Phase 7 — Developer Tooling

**Future possibilities:**

- VS Code extension.
- Improved command-line interface.
- Prebuilt releases for supported operating systems.
- Better debugging and diagnostics.
- Installation and usage documentation.

These milestones may evolve as the project develops.

## Technology Stack

- **Language:** C++
- **Build system:** CMake
- **Compiler integration:** Clang/LLVM, planned for a later phase
- **Testing:** CTest and project-specific tests
- **Development environment:** VS Code or another C++ IDE

## Getting Started

The phases are developed independently, and each phase may have its own requirements.

1. Open the README inside the phase you want to explore.
2. Install a compatible C++ compiler and CMake.
3. Follow that phase's build and test instructions.
4. Run its examples or demo programs.

For example, a phase using CMake can generally be configured and built with:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run these commands from the relevant phase's project directory. The exact commands may differ by phase and operating system.

## Current Limitations

Portable C++ is an experimental project. At this stage:

- It does not support the complete C++ language.
- A custom bytecode VM is not automatically compatible with existing native C++ binaries.
- Cross-platform behavior must be verified through testing.
- Operating-system APIs, external libraries, and C++ standard-library features require explicit compatibility work.
- The project is not intended to replace established C++ compilers at this stage.

## Contributing

Contributions, bug reports, suggestions, and constructive feedback are welcome.

Potential areas for contribution include:

- Virtual machine design and testing.
- Compiler frontend development.
- Bytecode format design.
- Cross-platform compatibility.
- Documentation and example programs.

Please describe the issue or proposed change clearly and include relevant test cases where possible.

## Long-Term Goal

The long-term goal is to investigate a practical workflow in which supported C++ source programs can be compiled into a portable representation and executed consistently across different operating systems using a compatible runtime.

The project will progress through incremental prototypes, testing, and clearly documented language support.

---

**Portable C++ — Exploring a more portable way to execute C++ programs.**
