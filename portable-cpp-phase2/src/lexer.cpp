#include "pcc/lexer.hpp"

#include <cstdio>
#include <limits>

namespace pcc {

namespace {

constexpr std::size_t kMaxLexErrors = 50;

bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_ident_start(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool is_ident_char(char c) { return is_ident_start(c) || is_digit(c); }

}  // namespace

LexResult lex(const std::string& s) {
    LexResult r;
    std::size_t   i = 0;
    std::uint32_t line = 1, col = 1;
    const std::size_t n = s.size();

    auto adv = [&]() {
        if (s[i] == '\n') { ++line; col = 1; } else { ++col; }
        ++i;
    };
    auto err = [&](SourceLoc loc, std::string msg) {
        r.diagnostics.push_back(Diagnostic{loc, std::move(msg)});
    };
    auto push = [&](TokenKind k, SourceLoc loc, std::string text, std::int64_t v = 0) {
        Token t;
        t.kind = k; t.loc = loc; t.text = std::move(text); t.int_value = v;
        r.tokens.push_back(std::move(t));
    };

    while (i < n) {
        if (r.diagnostics.size() >= kMaxLexErrors) {
            err(SourceLoc{line, col}, "too many errors; stopping");
            break;
        }
        const char c = s[i];
        const SourceLoc loc{line, col};

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v') { adv(); continue; }

        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            while (i < n && s[i] != '\n') adv();
            continue;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            adv(); adv();
            bool closed = false;
            while (i < n) {
                if (s[i] == '*' && i + 1 < n && s[i + 1] == '/') { adv(); adv(); closed = true; break; }
                adv();
            }
            if (!closed) err(loc, "unterminated block comment");
            continue;
        }

        if (is_digit(c)) {
            const std::size_t start = i;
            while (i < n && is_digit(s[i])) adv();
            const std::string digits = s.substr(start, i - start);
            if (i < n && is_ident_char(s[i])) {  // e.g. 10abc
                while (i < n && is_ident_char(s[i])) adv();
                err(loc, "invalid number literal '" + s.substr(start, i - start) + "'");
                continue;
            }
            if (digits.size() > 1 && digits[0] == '0') {
                err(loc, "leading zeros are not allowed in integer literals (C++ would read this as octal)");
                continue;
            }
            std::int64_t v = 0;
            bool overflow = false;
            for (char d : digits) {
                const int dv = d - '0';
                if (v > (std::numeric_limits<std::int64_t>::max() - dv) / 10) { overflow = true; break; }
                v = v * 10 + dv;
            }
            if (overflow) {
                err(loc, "integer literal is too large (maximum is 9223372036854775807)");
                continue;
            }
            push(TokenKind::Integer, loc, digits, v);
            continue;
        }

        if (is_ident_start(c)) {
            const std::size_t start = i;
            while (i < n && is_ident_char(s[i])) adv();
            push(TokenKind::Identifier, loc, s.substr(start, i - start));
            continue;
        }

        TokenKind k = TokenKind::Eof;
        switch (c) {
            case '+': k = TokenKind::Plus; break;
            case '-': k = TokenKind::Minus; break;
            case '*': k = TokenKind::Star; break;
            case '/': k = TokenKind::Slash; break;
            case '(': k = TokenKind::LParen; break;
            case ')': k = TokenKind::RParen; break;
            case ';': k = TokenKind::Semicolon; break;
            default: break;
        }
        if (k != TokenKind::Eof) {
            push(k, loc, std::string(1, c));
            adv();
            continue;
        }

        const unsigned char uc = static_cast<unsigned char>(c);
        if (uc >= 0x80) {  // consume one whole UTF-8 sequence, report once
            adv();
            while (i < n && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) adv();
            err(loc, "unexpected non-ASCII character (only ASCII is allowed outside comments)");
        } else if (uc >= 32 && uc < 127) {
            adv();
            err(loc, std::string("unexpected character '") + c + "'");
        } else {
            char buf[48];
            std::snprintf(buf, sizeof buf, "unexpected control character 0x%02X", static_cast<unsigned>(uc));
            adv();
            err(loc, buf);
        }
    }

    push(TokenKind::Eof, SourceLoc{line, col}, "");
    return r;
}

}  // namespace pcc
