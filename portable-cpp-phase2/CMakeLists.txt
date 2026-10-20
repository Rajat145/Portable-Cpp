cmake_minimum_required(VERSION 3.16)
project(PortableCpp VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PCC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

function(pcc_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
    if(PCC_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(PCC_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# Portable core: bytecode definitions + VM. No OS-specific code lives here.
add_library(pcc_core STATIC src/bytecode.cpp src/vm.cpp)
target_include_directories(pcc_core PUBLIC include)
pcc_enable_warnings(pcc_core)

# Compiler frontend: lexer, parser, AST, code generation. Depends only on pcc_core.
add_library(pcc_frontend STATIC
  src/diagnostics.cpp src/lexer.cpp src/ast.cpp
  src/parser.cpp src/codegen.cpp src/compiler.cpp)
target_include_directories(pcc_frontend PUBLIC include)
target_link_libraries(pcc_frontend PUBLIC pcc_core)
pcc_enable_warnings(pcc_frontend)

# Command-line driver.
add_executable(pcc src/pcc_main.cpp)
target_link_libraries(pcc PRIVATE pcc_frontend)
pcc_enable_warnings(pcc)

# Phase 1 demo (hand-written bytecode), kept for regression.
add_executable(pcc_phase1_demo src/main.cpp)
target_link_libraries(pcc_phase1_demo PRIVATE pcc_core)
pcc_enable_warnings(pcc_phase1_demo)

enable_testing()

add_executable(test_vm tests/test_vm.cpp)
target_link_libraries(test_vm PRIVATE pcc_core)
pcc_enable_warnings(test_vm)
add_test(NAME vm COMMAND test_vm)

add_executable(test_frontend tests/test_frontend.cpp)
target_link_libraries(test_frontend PRIVATE pcc_frontend)
pcc_enable_warnings(test_frontend)
add_test(NAME frontend COMMAND test_frontend)

# End-to-end tests through the real command-line tool.
add_test(NAME e2e_hello COMMAND pcc run ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.cpp)
set_tests_properties(e2e_hello PROPERTIES PASS_REGULAR_EXPRESSION "^30")
add_test(NAME e2e_bad_source COMMAND pcc run ${CMAKE_CURRENT_SOURCE_DIR}/examples/bad.cpp)
set_tests_properties(e2e_bad_source PROPERTIES WILL_FAIL TRUE)
