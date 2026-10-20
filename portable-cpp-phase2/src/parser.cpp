#include "pcc/parser.hpp"

#include <utility>

namespace pcc {

namespace {

constexpr std::uint32_t kMaxExprDepth = 512;  // bounds recursion and AST height

struct ParseError {
    Diagnostic diag;
};

std::string describe(const Token& t) {
    switch (t.kind) {
        case TokenKind::Eof:        return "end of input";
        case TokenKind::Integer:    return "number " + t.text;
        case TokenKind::Identifier: return "identifier '" + t.text + "'";
        default:                    return "'" + t.text + "'";
    }
}

class Parser {
public:
    explicit Parser(const std::vector<Token>& toks) : toks_(toks) {}

    ParseResult run() {
        ParseResult res;
        try {
            while (peek().kind != TokenKind::Eof) {
                res.program.statements.push_back(parse_statement());
            }
        } catch (const ParseError& e) {
            res.diagnostics.push_back(e.diag);
            res.program.statements.clear();
        }
        return res;
    }

private:
    const std::vector<Token>& toks_;
    std::size_t               pos_   = 0;
    std::uint32_t             depth_ = 0;

    const Token& peek() const {
        static const Token eof;
        return pos_ < toks_.size() ? toks_[pos_] : eof;
    }
    void advance() { if (pos_ < toks_.size()) ++pos_; }

    [[noreturn]] void fail(SourceLoc loc, std::string msg) const {
        throw ParseError{Diagnostic{loc, std::move(msg)}};
    }

    void expect(TokenKind k, const char* what) {
        if (peek().kind != k) {
            fail(peek().loc, std::string("expected ") + what + ", found " + describe(peek()));
        }
        advance();
    }

    struct DepthGuard {
        Parser& p;
        explicit DepthGuard(Parser& parser) : p(parser) {
            if (++p.depth_ > kMaxExprDepth) {
                p.fail(p.peek().loc, "expression nesting is too deep (limit " +
                                         std::to_string(kMaxExprDepth) + ")");
            }
        }
        ~DepthGuard() { --p.depth_; }
    };

    void check_height(const Expr& e) const {
        if (e.depth > kMaxExprDepth) {
            fail(e.loc, "expression is too complex (limit " + std::to_string(kMaxExprDepth) +
                            " levels)");
        }
    }

    PrintStmt parse_statement() {
        const Token& t = peek();
        if (t.kind == TokenKind::Identifier) {
            if (t.text != "print") {
                fail(t.loc, "unknown statement '" + t.text +
                                "' (only 'print(...);' is supported in this version)");
            }
            const SourceLoc loc = t.loc;
            advance();
            expect(TokenKind::LParen, "'(' after 'print'");
            auto e = parse_expr();
            expect(TokenKind::RParen, "')' after expression");
            expect(TokenKind::Semicolon, "';' after statement");
            PrintStmt s;
            s.loc = loc;
            s.value = std::move(e);
            return s;
        }
        fail(t.loc, "expected a statement, found " + describe(t));
    }

    std::unique_ptr<Expr> parse_expr() {
        DepthGuard g(*this);
        auto lhs = parse_term();
        while (peek().kind == TokenKind::Plus || peek().kind == TokenKind::Minus) {
            const Token op = peek();
            advance();
            auto rhs = parse_term();
            lhs = make_binary(op.loc, op.kind == TokenKind::Plus ? BinOp::Add : BinOp::Sub,
                              std::move(lhs), std::move(rhs));
            check_height(*lhs);
        }
        return lhs;
    }

    std::unique_ptr<Expr> parse_term() {
        auto lhs = parse_unary();
        while (peek().kind == TokenKind::Star || peek().kind == TokenKind::Slash) {
            const Token op = peek();
            advance();
            auto rhs = parse_unary();
            lhs = make_binary(op.loc, op.kind == TokenKind::Star ? BinOp::Mul : BinOp::Div,
                              std::move(lhs), std::move(rhs));
            check_height(*lhs);
        }
        return lhs;
    }

    std::unique_ptr<Expr> parse_unary() {
        DepthGuard g(*this);
        if (peek().kind == TokenKind::Minus) {
            const SourceLoc loc = peek().loc;
            advance();
            auto e = make_negate(loc, parse_unary());
            check_height(*e);
            return e;
        }
        return parse_primary();
    }

    std::unique_ptr<Expr> parse_primary() {
        const Token& t = peek();
        if (t.kind == TokenKind::Integer) {
            auto e = make_int(t.loc, t.int_value);
            advance();
            return e;
        }
        if (t.kind == TokenKind::LParen) {
            advance();
            auto e = parse_expr();
            expect(TokenKind::RParen, "')'");
            return e;
        }
        fail(t.loc, "expected expression, found " + describe(t));
    }
};

}  // namespace

ParseResult parse(const std::vector<Token>& tokens) {
    return Parser(tokens).run();
}

}  // namespace pcc
