// 文件职责：把源文本切分为 token，供 parser 构建 AST。本文件负责声明接口和数据结构。
#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>

// ============================================================
// Token 类型枚举
// SysY2022 完整词法单元
// ============================================================
enum class TK {
    // 关键字
    Int, Float, Char, Void, Const, Static,
    If, Else, While, Do, Switch, Case, Default, Break, Continue, Return, Printf,
    // 字面量
    IntLit,     // 整型字面量：十进制/十六进制/八进制
    FloatLit,   // 浮点字面量（2025年新增 float 支持）
    CharLit,
    StringLit,
    // 标识符
    Id,
    // 算术运算符
    Plus, Minus, Star, Slash, Percent,   // + - * / %
    // 关系运算符
    Eq, Ne, Lt, Le, Gt, Ge,             // == != < <= > >=
    // 逻辑运算符
    LAnd, LOr, LNot,                    // && || !
    // 按位运算符
    BitAnd, BitOr, BitXor, BitNot,      // & | ^ ~
    // 赋值
    Assign,                             // =
    // 标点符号
    LParen, RParen,                     // ( )
    LBrace, RBrace,                     // { }
    LBracket, RBracket,                 // [ ]
    Comma, Semi, Colon,                 // , ; :
    // 结束
    Eof
};

// ============================================================
// Token 结构
// ============================================================
struct Token {
    TK      type;
    std::string text;   // 原始文本（用于错误信息和标识符名称）
    int     line;       // 行号（从1开始）
    int32_t ival;       // 整型字面量的值
    float   fval;       // 浮点字面量的值

    Token(TK t, std::string s, int ln)
        : type(t), text(std::move(s)), line(ln), ival(0), fval(0.0f) {}
};

struct LexDiagnostic {
    int line = 0;
    char code = 'a';
};

// ============================================================
// 词法分析器
// 功能：将源码字符串转换为 Token 序列
// ============================================================
class Lexer {
public:
    explicit Lexer(const std::string& source, std::string filename = "<input>");

    // 获取下一个 token（消费）
    Token next();

    // 预览下一个 token（不消费）
    const Token& peek();

    // 是否到达文件末尾
    bool atEof() { return peek().type == TK::Eof; }

    int curLine() const { return line_; }
    const std::vector<LexDiagnostic>& diagnostics() const { return diagnostics_; }

private:
    std::string src_;        // 源码
    std::string filename_;
    size_t pos_;             // 当前扫描位置
    int line_;               // 当前行号

    // peek 缓存
    bool hasCached_;
    Token cached_;
    std::vector<LexDiagnostic> diagnostics_;

    // 跳过空白和注释（// 行注释 和 /* */ 块注释）
    void skipWS();

    // 读取一个 token（内部）
    Token readToken();

    // 读取数字（整数或浮点数）
    Token readNumber();

    Token readCharLiteral();
    Token readStringLiteral();

    // 读取标识符或关键字
    Token readIdOrKw();

    char cur() const {
        return pos_ < src_.size() ? src_[pos_] : '\0';
    }
    char peek1() const {
        return (pos_ + 1) < src_.size() ? src_[pos_ + 1] : '\0';
    }
    char advance() {
        char c = src_[pos_++];
        if (c == '\n') line_++;
        return c;
    }
    bool match(char c) {
        if (cur() == c) { advance(); return true; }
        return false;
    }
};
