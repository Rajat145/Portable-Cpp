#include "pcc/codegen.hpp"

#include "pcc/bytecode.hpp"

namespace pcc {

namespace {

void gen_expr(const Expr& e, CodeBuilder& b) {
    switch (e.kind) {
        case ExprKind::IntLiteral:
            b.push_i64(e.value);
            break;
        case ExprKind::Negate:  // -x is defined as 0 - x (wrapping)
            b.push_i64(0);
            gen_expr(*e.lhs, b);
            b.emit(Op::Sub);
            break;
        case ExprKind::Binary:
            gen_expr(*e.lhs, b);
            gen_expr(*e.rhs, b);
            switch (e.op) {
                case BinOp::Add: b.emit(Op::Add); break;
                case BinOp::Sub: b.emit(Op::Sub); break;
                case BinOp::Mul: b.emit(Op::Mul); break;
                case BinOp::Div: b.emit(Op::Div); break;
            }
            break;
    }
}

}  // namespace

std::vector<std::uint8_t> generate_code(const Program& program) {
    CodeBuilder b;
    for (const PrintStmt& s : program.statements) {
        gen_expr(*s.value, b);
        b.emit(Op::Print);
    }
    b.emit(Op::Halt);
    return b.bytes();
}

}  // namespace pcc
