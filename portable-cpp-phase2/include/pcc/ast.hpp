// Abstract syntax tree (Phase 2).
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "pcc/diagnostics.hpp"

namespace pcc {

enum class ExprKind { IntLiteral, Negate, Binary };
enum class BinOp { Add, Sub, Mul, Div };

struct Expr {
    ExprKind              kind  = ExprKind::IntLiteral;
    SourceLoc             loc;              // literal start, or operator position
    std::int64_t          value = 0;        // IntLiteral
    BinOp                 op    = BinOp::Add;  // Binary
    std::unique_ptr<Expr> lhs;              // Binary lhs, Negate operand
    std::unique_ptr<Expr> rhs;              // Binary rhs
    std::uint32_t         depth = 1;        // tree height; bounded by the parser
};

struct PrintStmt {
    SourceLoc             loc;
    std::unique_ptr<Expr> value;
};

struct Program {
    std::vector<PrintStmt> statements;
};

std::unique_ptr<Expr> make_int(SourceLoc loc, std::int64_t value);
std::unique_ptr<Expr> make_negate(SourceLoc loc, std::unique_ptr<Expr> operand);
std::unique_ptr<Expr> make_binary(SourceLoc loc, BinOp op, std::unique_ptr<Expr> l,
                                  std::unique_ptr<Expr> r);

// S-expression form, e.g. "(+ 10 (* 2 3))" and "(neg 5)". For tests/debugging.
std::string dump(const Expr& e);

}  // namespace pcc
