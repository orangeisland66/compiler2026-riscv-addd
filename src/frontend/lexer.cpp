// 文件职责：把源文本切分为 token，供 parser 构建 AST。本文件负责实现具体逻辑。
#include "lexer.h"
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <unordered_map>
#include <cmath>

// ============================================================
// 关键字表
// ============================================================
static const std::unordered_map<std::string, TK> KEYWORDS = {
    {"int",      TK::Int},
    {"float",    TK::Float},
    {"char",     TK::Char},
    {"void",     TK::Void},
    {"const",    TK::Const},
    {"static",   TK::Static},
    {"if",       TK::If},
    {"else",     TK::Else},
    {"while",    TK::While},
    {"do",       TK::Do},
    {"switch",   TK::Switch},
    {"case",     TK::Case},
    {"default",  TK::Default},
    {"break",    TK::Break},
    {"continue", TK::Continue},
    {"return",   TK::Return},
    {"printf",   TK::Printf},
};

Lexer::Lexer(const std::string& source, std::string filename)
    : src_(source), filename_(std::move(filename)),
      pos_(0), line_(1), hasCached_(false), cached_(TK::Eof, "", 0) {}

// ============================================================
// peek: 不消费地查看下一个 token
// ============================================================
const Token& Lexer::peek() {
    if (!hasCached_) {
        cached_ = readToken();
        hasCached_ = true;
    }
    return cached_;
}

// ============================================================
// next: 消费并返回下一个 token
// ============================================================
Token Lexer::next() {
    if (hasCached_) {
        hasCached_ = false;
        return cached_;
    }
    return readToken();
}

// ============================================================
// skipWS: 跳过空白字符和注释
// ============================================================
void Lexer::skipWS() {
    while (pos_ < src_.size()) {
        char c = cur();
        if (std::isspace(c)) {
            advance();
        } else if (c == '/' && peek1() == '/') {
            // 单行注释：跳到行尾
            while (pos_ < src_.size() && cur() != '\n') advance();
        } else if (c == '/' && peek1() == '*') {
            // 块注释：找到 */
            advance(); advance(); // 消费 /*
            while (pos_ < src_.size()) {
                if (cur() == '*' && peek1() == '/') {
                    advance(); advance();
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

// ============================================================
// readNumber: 读取整型或浮点字面量
// SysY2022 支持：
//   整型：十进制、0x前缀十六进制、0前缀八进制
//   浮点：十进制小数/指数形式、十六进制浮点 0x...p...
// ============================================================
Token Lexer::readNumber() {
    size_t start = pos_;
    int startLine = line_;
    bool isFloat = false;

    if (cur() == '0' && (peek1() == 'x' || peek1() == 'X')) {
        // 十六进制
        advance(); advance(); // 消费 0x
        while (std::isxdigit(cur()) || cur() == '_') advance();

        // 检查是否是十六进制浮点数（0x...p...）
        if (cur() == '.') {
            isFloat = true;
            advance();
            while (std::isxdigit(cur())) advance();
        }
        if (cur() == 'p' || cur() == 'P') {
            isFloat = true;
            advance();
            if (cur() == '+' || cur() == '-') advance();
            while (std::isdigit(cur())) advance();
        }
    } else {
        // 十进制或八进制
        while (std::isdigit(cur())) advance();

        // 浮点：小数点或指数
        if (cur() == '.') {
            isFloat = true;
            advance();
            while (std::isdigit(cur())) advance();
        }
        if (cur() == 'e' || cur() == 'E') {
            isFloat = true;
            advance();
            if (cur() == '+' || cur() == '-') advance();
            while (std::isdigit(cur())) advance();
        }
    }

    std::string text = src_.substr(start, pos_ - start);

    if (isFloat) {
        Token tok(TK::FloatLit, text, startLine);
        tok.fval = std::stof(text);
        return tok;
    } else {
        Token tok(TK::IntLit, text, startLine);
        // 使用 strtol 正确处理 0x 十六进制和 0 八进制前缀
        tok.ival = (int32_t)std::strtol(text.c_str(), nullptr, 0);
        return tok;
    }
}

// ============================================================
// readIdOrKw: 读取标识符或关键字
// ============================================================
Token Lexer::readIdOrKw() {
    size_t start = pos_;
    int startLine = line_;
    while (std::isalnum(cur()) || cur() == '_') advance();
    std::string text = src_.substr(start, pos_ - start);

    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        return Token(it->second, text, startLine);
    }
    return Token(TK::Id, text, startLine);
}

namespace {

char decodeCharEscape(char c, int line) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '0': return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        default:
            throw std::runtime_error("Line " + std::to_string(line) +
                                     ": invalid character escape");
    }
}

} // namespace

Token Lexer::readCharLiteral() {
    const int startLine = line_;
    advance(); // opening quote
    if (pos_ >= src_.size() || cur() == '\n' || cur() == '\'')
        throw std::runtime_error("Line " + std::to_string(startLine) +
                                 ": invalid character literal");

    unsigned char value = 0;
    if (cur() == '\\') {
        advance();
        if (pos_ >= src_.size())
            throw std::runtime_error("Line " + std::to_string(startLine) +
                                     ": unterminated character literal");
        value = static_cast<unsigned char>(decodeCharEscape(cur(), startLine));
        advance();
    } else {
        const unsigned char c = static_cast<unsigned char>(cur());
        if (c < 0x20 || c > 0x7e || c == '\\')
            throw std::runtime_error("Line " + std::to_string(startLine) +
                                     ": invalid character literal");
        value = c;
        advance();
    }
    if (pos_ >= src_.size() || cur() != '\'')
        throw std::runtime_error("Line " + std::to_string(startLine) +
                                 ": unterminated character literal");
    advance();
    Token token(TK::CharLit, "", startLine);
    token.ival = value;
    return token;
}

Token Lexer::readStringLiteral() {
    const int startLine = line_;
    advance(); // opening quote
    std::string decoded;
    while (pos_ < src_.size() && cur() != '"') {
        if (cur() == '\n' || cur() == '\r')
            throw std::runtime_error("Line " + std::to_string(startLine) +
                                     ": unterminated string literal");
        if (cur() == '\\') {
            advance();
            if (pos_ >= src_.size() || cur() != 'n')
                throw std::runtime_error("Line " + std::to_string(startLine) +
                                         ": invalid string escape");
            decoded.push_back('\n');
            advance();
            continue;
        }
        if (cur() == '%') {
            if (peek1() != 'd' && peek1() != 'c' && peek1() != 's')
                throw std::runtime_error("Line " + std::to_string(startLine) +
                                         ": invalid format character");
            decoded.push_back('%');
            decoded.push_back(peek1());
            advance();
            advance();
            continue;
        }
        const unsigned char c = static_cast<unsigned char>(cur());
        if (!(c == 32 || c == 33 || (c >= 40 && c <= 126)))
            throw std::runtime_error("Line " + std::to_string(startLine) +
                                     ": invalid character in string literal");
        decoded.push_back(static_cast<char>(c));
        advance();
    }
    if (pos_ >= src_.size())
        throw std::runtime_error("Line " + std::to_string(startLine) +
                                 ": unterminated string literal");
    advance();
    return Token(TK::StringLit, std::move(decoded), startLine);
}

// ============================================================
// readToken: 读取一个 token 的核心逻辑
// ============================================================
Token Lexer::readToken() {
    skipWS();

    if (pos_ >= src_.size()) {
        return Token(TK::Eof, "", line_);
    }

    int startLine = line_;
    char c = cur();

    if (c == '\'') return readCharLiteral();
    if (c == '"') return readStringLiteral();

    // 数字字面量
    if (std::isdigit(c) || (c == '.' && std::isdigit(peek1()))) {
        return readNumber();
    }

    // 标识符或关键字
    if (std::isalpha(c) || c == '_') {
        return readIdOrKw();
    }

    // 运算符和标点
    advance();
    switch (c) {
        case '+': return Token(TK::Plus,    "+", startLine);
        case '-': return Token(TK::Minus,   "-", startLine);
        case '*': return Token(TK::Star,    "*", startLine);
        case '/': return Token(TK::Slash,   "/", startLine);
        case '%': return Token(TK::Percent, "%", startLine);
        case '(': return Token(TK::LParen,  "(", startLine);
        case ')': return Token(TK::RParen,  ")", startLine);
        case '{': return Token(TK::LBrace,  "{", startLine);
        case '}': return Token(TK::RBrace,  "}", startLine);
        case '[': return Token(TK::LBracket,"[", startLine);
        case ']': return Token(TK::RBracket,"]", startLine);
        case ',': return Token(TK::Comma,   ",", startLine);
        case ';': return Token(TK::Semi,    ";", startLine);
        case ':': return Token(TK::Colon,   ":", startLine);
        case '!':
            if (match('=')) return Token(TK::Ne,   "!=", startLine);
            return Token(TK::LNot, "!", startLine);
        case '=':
            if (match('=')) return Token(TK::Eq,   "==", startLine);
            return Token(TK::Assign, "=", startLine);
        case '<':
            if (match('=')) return Token(TK::Le,   "<=", startLine);
            return Token(TK::Lt, "<", startLine);
        case '>':
            if (match('=')) return Token(TK::Ge,   ">=", startLine);
            return Token(TK::Gt, ">", startLine);
        case '&':
            if (match('&')) return Token(TK::LAnd, "&&", startLine);
            return Token(TK::BitAnd, "&", startLine);
        case '|':
            if (match('|')) return Token(TK::LOr,  "||", startLine);
            return Token(TK::BitOr, "|", startLine);
        case '^': return Token(TK::BitXor, "^", startLine);
        case '~': return Token(TK::BitNot, "~", startLine);
        default:
            throw std::runtime_error("Line " + std::to_string(startLine) +
                                     ": unexpected char '" + c + "'");
    }
}
