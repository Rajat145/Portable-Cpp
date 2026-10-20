#include "pcc/ast.hpp"

#include <algorithm>
#include <utility>

namespace pcc {

std::unique_ptr<Expr> make_int(SourceLoc loc, std::int64_t value) {
    auto e = std::make_unique<Expr>();
    e->kind = ExprKind::IntLiteral;
    e->loc = loc;
    e->value = value;
    return e;
}

std::unique_ptr<Expr> make_negate(SourceLoc loc, std::unique_ptr<Expr> operand) {
    auto e = std::make_unique<Expr>();
    e->kind = ExprKind::Negate;
    e->loc = loc;
    e->depth = operand->depth + 1;
    e->lhs = std::move(operand);
    return e;
}

std::unique_ptr<Expr> make_binary(SourceLoc loc, BinOp op, std::unique_ptr<Expr> l,
                                  std::unique_ptr<Expr> r) {
    auto e = std::make_unique<Expr>();
    e->kind = ExprKind::Binary;
    e->loc = loc;
    e->op = op;
    e->depth = std::max(l->depth, r->depth) + 1;
    e->lhs = std::move(l);
    e->rhs = std::move(r);
    return e;
}

std::string dump(const Expr& e) {
    switch (e.kind) {
        case ExprKind::IntLiteral:
            return std::to_string(e.value);
        case ExprKind::Negate:
            return "(neg " + dump(*e.lhs) + ")";
        case ExprKind::Binary: {
            const char* sym = "?";
            switch (e.op) {
                case BinOp::Add: sym = "+"; break;
                case BinOp::Sub: sym = "-"; break;
                case BinOp::Mul: sym = "*"; break;
                case BinOp::Div: sym = "/"; break;
            }
            return std::string("(") + sym + " " + dump(*e.lhs) + " " + dump(*e.rhs) + ")";
        }
    }
    return "?";
}

}  // namespace pcc
