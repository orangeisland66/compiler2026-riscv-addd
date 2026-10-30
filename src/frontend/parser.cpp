// 文件职责：解析 SysY 语法并构建带基础语义信息的 AST。本文件负责实现具体逻辑。
#include "parser.h"
#include <sstream>
#include <climits>

// tryEvalDim 返回值：INT32_MIN 表示"无法在编译期求值"
static const int32_t EVAL_UNKNOWN = INT32_MIN;

// ============================================================
// 辅助方法
// ============================================================
Parser::Parser(Lexer& lex) : lex_(lex) {}

void Parser::error(const std::string& msg) {
    throw std::runtime_error("Line " + std::to_string(peek().line) + ": " + msg);
}

Token Parser::expect(TK t, const char* msg) {
    if (!check(t)) {
        char code = 0;
        if (t == TK::Semi) code = 'i';
        if (t == TK::RParen) code = 'j';
        if (t == TK::RBracket) code = 'k';
        if (code != 0) {
            diagnostics_.push_back({peek().line, code});
            return Token(t, "", peek().line);
        }
        error(msg ? std::string(msg)
                  : "expected token, got '" + peek().text + "'");
    }
    return next();
}

bool Parser::tryConsume(TK t) {
    if (check(t)) { next(); return true; }
    return false;
}

std::string Parser::expectId() {
    if (!check(TK::Id)) error("expected identifier, got '" + peek().text + "'");
    return next().text;
}

BaseType Parser::parseBaseType() {
    if (tryConsume(TK::Int))   return BaseType::Int;
    if (tryConsume(TK::Float)) return BaseType::Float;
    if (tryConsume(TK::Char))  return BaseType::Char;
    if (tryConsume(TK::Void))  return BaseType::Void;
    error("expected type keyword");
    return BaseType::Int;
}

// ============================================================
// CompUnit → (Decl | FuncDef)*
// ============================================================
std::unique_ptr<CompUnit> Parser::parseCompUnit() {
    auto cu = std::make_unique<CompUnit>();
    cu->line = 1;

    while (!check(TK::Eof)) {
        cu->items.push_back(parseTopLevel());
    }
    return cu;
}

// ============================================================
// 顶层元素：通过前瞻区分全局变量声明和函数定义
// 文法：
//   FuncDef → FuncType Ident '(' ...
//   Decl    → ['const'] BType Ident '[' | ',' | '=' | ';'
// 策略：解析 [const] type ident，然后看下一个 token：
//   '(' → 函数定义
//   其他 → 变量声明
// ============================================================
CompUnit::Item Parser::parseTopLevel() {
    CompUnit::Item item;
    bool isConst = tryConsume(TK::Const);

    // 解析类型
    if (!checkType()) error("expected type keyword at top level");
    BaseType base = parseBaseType();

    // 解析第一个标识符
    std::string name = expectId();

    if (!isConst && check(TK::LParen)) {
        // 函数定义
        SysType retType; retType.base = base;
        item.kind = CompUnit::Item::Kind::FuncDef;
        item.func = parseFuncDef(retType, name);
    } else {
        // 全局变量/常量声明
        // 需要回退：我们已经消费了 type 和 name
        // 构造第一个 VarDef，然后继续解析剩余的
        item.kind = CompUnit::Item::Kind::GlobalDecl;
        item.decl = std::make_unique<DeclStmt>();
        item.decl->isConst = isConst;
        item.decl->baseType = base;

        // 解析第一个变量定义（name 已经读出）
        auto firstDef = std::make_unique<VarDef>();
        firstDef->name = name;
        firstDef->isConst = isConst;
        firstDef->type.base = base;

        // 数组维度（支持 const int 变量作为维度，如 int arr[N]）
        while (check(TK::LBracket)) {
            next();
            auto dimExpr = parseConstExpr();
            expect(TK::RBracket);
            int32_t dim = tryEvalDim(dimExpr.get());
            // 维度必须为正整数；EVAL_UNKNOWN(INT32_MIN) 或 <=0 的值视为未知
            firstDef->type.dims.push_back((dim != EVAL_UNKNOWN && dim > 0) ? dim : 0);
        }

        // 初始化值
        if (tryConsume(TK::Assign)) {
            firstDef->initVal = parseInitVal();
            // 记录 const int 标量值到编译期常量表（包括负值，如 const int N=-1）
            if (isConst && base == BaseType::Int && !firstDef->type.isArray()) {
                if (auto* sc = dynamic_cast<ScalarInit*>(firstDef->initVal.get())) {
                    int32_t v = tryEvalDim(sc->expr.get());
                    if (v != EVAL_UNKNOWN)
                        constInts_[firstDef->name] = v;
                }
            }
        }
        item.decl->defs.push_back(std::move(firstDef));

        // 继续解析同一行的其他声明
        while (tryConsume(TK::Comma)) {
            item.decl->defs.push_back(parseVarDef(isConst, base));
        }
        expect(TK::Semi, "expected ';' after declaration");
    }
    return item;
}

// ============================================================
// 函数定义
// FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
// ============================================================
std::unique_ptr<FuncDef> Parser::parseFuncDef(SysType retType, std::string name) {
    auto func = std::make_unique<FuncDef>();
    func->line = peek().line;
    func->retType = retType;
    func->name = name;

    expect(TK::LParen);
    // 参数列表（可为空）
    if (!check(TK::RParen)) {
        func->params.push_back(parseFuncParam());
        while (tryConsume(TK::Comma)) {
            func->params.push_back(parseFuncParam());
        }
    }
    expect(TK::RParen);
    func->body = parseBlock();
    return func;
}

// FuncFParam → BType Ident ['[' ']' ('[' ConstExp ']')*]
std::unique_ptr<FuncParam> Parser::parseFuncParam() {
    auto param = std::make_unique<FuncParam>();
    param->line = peek().line;

    // 类型
    if (!checkType()) error("expected type in parameter");
    BaseType base = parseBaseType();
    param->type.base = base;

    param->name = expectId();

    // 数组参数：第一维可省略 []，后续维必须有大小
    if (check(TK::LBracket)) {
        next();
        expect(TK::RBracket); // 第一维省略
        param->type.dims.push_back(-1); // -1 表示第一维省略（指针）

        // 后续维：与 parseVarDef 一致，支持 const int 变量作为维度
        while (check(TK::LBracket)) {
            next();
            auto dimExpr = parseConstExpr();
            expect(TK::RBracket);
            int32_t dim = tryEvalDim(dimExpr.get());
            param->type.dims.push_back((dim != EVAL_UNKNOWN && dim > 0) ? dim : 0);
        }
    }
    return param;
}

// ============================================================
// 声明语句
// ConstDecl → 'const' BType ConstDef (',' ConstDef)* ';'
// VarDecl   → BType VarDef (',' VarDef)* ';'
// ============================================================
std::unique_ptr<DeclStmt> Parser::parseDecl(bool isConst, BaseType base,
                                            bool isStatic) {
    auto decl = std::make_unique<DeclStmt>();
    decl->isConst = isConst;
    decl->isStatic = isStatic;
    decl->baseType = base;

    decl->defs.push_back(parseVarDef(isConst, base));
    while (tryConsume(TK::Comma)) {
        decl->defs.push_back(parseVarDef(isConst, base));
    }
    expect(TK::Semi, "expected ';' after declaration");
    return decl;
}

// ============================================================
// tryEvalDim: 尝试在解析时对维度表达式求值
// 返回 INT32_MIN 表示"无法在编译期确定"（用 UNKNOWN 区分负值与失败）
// 支持：整型字面量、已知 const int 变量（包括负值）、简单算术
// ============================================================
int32_t Parser::tryEvalDim(Expr* e) {
    if (auto* lit = dynamic_cast<IntLitExpr*>(e))
        return lit->val;
    if (auto* lv = dynamic_cast<LValExpr*>(e)) {
        if (lv->indices.empty()) {
            auto it = constInts_.find(lv->name);
            if (it != constInts_.end()) return it->second;
        }
    }
    if (auto* un = dynamic_cast<UnaryExpr*>(e)) {
        int32_t v = tryEvalDim(un->operand.get());
        if (v == EVAL_UNKNOWN) return EVAL_UNKNOWN;
        if (un->op == '-') return -v;
        if (un->op == '+') return v;
        if (un->op == '!') return !v;
        if (un->op == '~') return ~v;
    }
    if (auto* bin = dynamic_cast<BinaryExpr*>(e)) {
        int32_t l = tryEvalDim(bin->lhs.get());
        int32_t r = tryEvalDim(bin->rhs.get());
        if (l == EVAL_UNKNOWN || r == EVAL_UNKNOWN) return EVAL_UNKNOWN;
        if (bin->op == "+") return l + r;
        if (bin->op == "-") return l - r;
        if (bin->op == "*") return l * r;
        if (bin->op == "/" && r != 0) return l / r;
        if (bin->op == "%" && r != 0) return l % r;
        if (bin->op == "&") return l & r;
        if (bin->op == "|") return l | r;
        if (bin->op == "^") return l ^ r;
    }
    return EVAL_UNKNOWN; // 无法在解析时确定
}

// VarDef → Ident ('[' ConstExp ']')* ['=' InitVal]
std::unique_ptr<VarDef> Parser::parseVarDef(bool isConst, BaseType base) {
    auto def = std::make_unique<VarDef>();
    def->line = peek().line;
    def->isConst = isConst;
    def->type.base = base;
    def->name = expectId();

    // 数组维度：尝试在解析时求值（支持 const int 变量作为维度）
    while (check(TK::LBracket)) {
        next();
        auto dimExpr = parseConstExpr();
        expect(TK::RBracket);
        int32_t dim = tryEvalDim(dimExpr.get());
        def->type.dims.push_back((dim != EVAL_UNKNOWN && dim > 0) ? dim : 0);
    }

    // 初始化值
    if (tryConsume(TK::Assign)) {
        def->initVal = parseInitVal();
        // 如果是 const int 标量，记录其值（含负数）到 constInts_
        if (isConst && base == BaseType::Int && !def->type.isArray()) {
            if (auto* sc = dynamic_cast<ScalarInit*>(def->initVal.get())) {
                int32_t v = tryEvalDim(sc->expr.get());
                if (v != EVAL_UNKNOWN) constInts_[def->name] = v;
                // 整型字面量直接记录（保证精度）
                if (auto* lit = dynamic_cast<IntLitExpr*>(sc->expr.get()))
                    constInts_[def->name] = lit->val;
            }
        }
    } else if (isConst) {
        error("const variable '" + def->name + "' must be initialized");
    }
    return def;
}

// InitVal → Exp | '{' [InitVal (',' InitVal)*] '}'
InitValPtr Parser::parseInitVal() {
    if (check(TK::StringLit)) {
        auto token = next();
        auto init = std::make_unique<StringInit>();
        init->line = token.line;
        init->value = std::move(token.text);
        while (check(TK::StringLit)) init->value += next().text;
        return init;
    }
    if (check(TK::LBrace)) {
        next();
        auto agg = std::make_unique<AggregateInit>();
        agg->line = peek().line;
        if (!check(TK::RBrace)) {
            agg->items.push_back(parseInitVal());
            while (tryConsume(TK::Comma)) {
                if (check(TK::RBrace)) break; // 允许尾部逗号
                agg->items.push_back(parseInitVal());
            }
        }
        expect(TK::RBrace, "expected '}' in initializer");
        return agg;
    }
    // 标量初始化
    auto scalar = std::make_unique<ScalarInit>();
    scalar->line = peek().line;
    scalar->expr = parseExpr();
    return scalar;
}

// ============================================================
// 语句解析
// ============================================================
std::unique_ptr<BlockStmt> Parser::parseBlock() {
    auto block = std::make_unique<BlockStmt>();
    block->line = peek().line;
    expect(TK::LBrace, "expected '{'");

    while (!check(TK::RBrace) && !check(TK::Eof)) {
        BlockStmt::Item item;

        // 判断是声明还是语句
        bool isDecl = false;
        if (check(TK::Const) || check(TK::Static)) {
            isDecl = true;
        } else if (check(TK::Int) || check(TK::Float) || check(TK::Char)) {
            // 可能是声明或者以类型关键字开头的表达式（SysY中没有这种情况）
            isDecl = true;
        }

        if (isDecl) {
            bool isConst = tryConsume(TK::Const);
            bool isStatic = tryConsume(TK::Static);
            if (!checkType()) error("expected type keyword in declaration");
            BaseType base = parseBaseType();
            item.kind = BlockStmt::Item::Kind::Decl;
            item.decl = parseDecl(isConst, base, isStatic);
        } else {
            item.kind = BlockStmt::Item::Kind::Stmt;
            item.stmt = parseStmt();
        }
        block->items.push_back(std::move(item));
    }
    expect(TK::RBrace, "expected '}'");
    return block;
}

StmtPtr Parser::parseStmt() {
    int ln = peek().line;

    // 空语句
    if (check(TK::Semi)) {
        auto s = std::make_unique<ExprStmt>();
        s->line = ln;
        next();
        return s;
    }

    // 块语句
    if (check(TK::LBrace)) {
        return parseBlock();
    }

    // if 语句
    if (check(TK::If)) {
        next();
        auto s = std::make_unique<IfStmt>();
        s->line = ln;
        expect(TK::LParen);
        s->cond = parseCond();
        expect(TK::RParen);
        s->thenBranch = parseStmt();
        if (tryConsume(TK::Else)) {
            s->elseBranch = parseStmt();
        }
        return s;
    }

    // while 语句
    if (check(TK::While)) {
        next();
        auto s = std::make_unique<WhileStmt>();
        s->line = ln;
        expect(TK::LParen);
        s->cond = parseCond();
        expect(TK::RParen);
        s->body = parseStmt();
        return s;
    }

    // do-while 语句：先生成循环体，再在尾部检查条件。
    if (check(TK::Do)) {
        next();
        auto s = std::make_unique<DoWhileStmt>();
        s->line = ln;
        s->body = parseStmt();
        expect(TK::While);
        expect(TK::LParen);
        s->cond = parseCond();
        expect(TK::RParen);
        expect(TK::Semi);
        return s;
    }

    if (check(TK::Switch)) {
        next();
        auto s = std::make_unique<SwitchStmt>();
        s->line = ln;
        expect(TK::LParen);
        s->value = parseExpr();
        expect(TK::RParen);
        expect(TK::LBrace);
        while (!check(TK::RBrace) && !check(TK::Eof)) {
            SwitchStmt::Clause clause;
            clause.line = peek().line;
            if (tryConsume(TK::Case)) {
                if (!check(TK::IntLit) && !check(TK::CharLit))
                    error("case value must be an integer or character literal");
                clause.value = parsePrimaryExpr();
            } else if (tryConsume(TK::Default)) {
                clause.isDefault = true;
            } else {
                error("expected 'case' or 'default' in switch");
            }
            expect(TK::Colon);
            while (!check(TK::Case) && !check(TK::Default) &&
                   !check(TK::RBrace) && !check(TK::Eof))
                clause.statements.push_back(parseStmt());
            s->clauses.push_back(std::move(clause));
        }
        expect(TK::RBrace);
        return s;
    }

    if (check(TK::Printf)) {
        next();
        auto s = std::make_unique<PrintfStmt>();
        s->line = ln;
        expect(TK::LParen);
        if (!check(TK::StringLit)) error("printf requires a format string");
        s->format = next().text;
        while (tryConsume(TK::Comma)) s->args.push_back(parseExpr());
        expect(TK::RParen);
        expect(TK::Semi);
        return s;
    }

    // break
    if (check(TK::Break)) {
        next();
        expect(TK::Semi);
        auto s = std::make_unique<BreakStmt>();
        s->line = ln;
        return s;
    }

    // continue
    if (check(TK::Continue)) {
        next();
        expect(TK::Semi);
        auto s = std::make_unique<ContinueStmt>();
        s->line = ln;
        return s;
    }

    // return
    if (check(TK::Return)) {
        next();
        auto s = std::make_unique<ReturnStmt>();
        s->line = ln;
        if (!check(TK::Semi)) {
            s->val = parseExpr();
        }
        expect(TK::Semi);
        return s;
    }

    // 赋值语句或表达式语句
    // 策略：先解析 Exp，如果后面是 '='，则是赋值语句
    // 注意：SysY 左值必须是 LVal（变量名+可选下标），不是任意表达式
    ExprPtr expr = parseExpr();
    if (check(TK::Assign)) {
        next();
        // expr 必须是 LValExpr
        if (!dynamic_cast<LValExpr*>(expr.get())) {
            error("left hand side of assignment must be a variable");
        }
        auto s = std::make_unique<AssignStmt>();
        s->line = ln;
        s->lval = std::move(expr);
        s->rhs  = parseExpr();
        expect(TK::Semi);
        return s;
    }

    // 表达式语句
    auto s = std::make_unique<ExprStmt>();
    s->line = ln;
    s->expr = std::move(expr);
    expect(TK::Semi);
    return s;
}

// ============================================================
// 表达式解析（优先级从低到高）
// 扩展表达式允许关系、按位和逻辑运算产生整数值。
// ============================================================
// The established SysY corpus also uses condition-valued expressions in
// assignments and arithmetic contexts, so keep Exp a superset of AddExp.
ExprPtr Parser::parseExpr()    { return parseCond(); }
ExprPtr Parser::parseCond() {
    return parseLOrExpr();
}
ExprPtr Parser::parseConstExpr() { return parseAddExpr(); } // ConstExp 限制为 AddExp

// LOrExp → LAndExp ('||' LAndExp)*
ExprPtr Parser::parseLOrExpr() {
    auto lhs = parseLAndExpr();
    while (check(TK::LOr)) {
        int ln = peek().line;
        next();
        auto rhs = parseLAndExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = "||";
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// LAndExp → BitOrExp ('&&' BitOrExp)*
ExprPtr Parser::parseLAndExpr() {
    auto lhs = parseBitOrExpr();
    while (check(TK::LAnd)) {
        int ln = peek().line;
        next();
        auto rhs = parseBitOrExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = "&&";
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// BitOrExp → BitXorExp ('|' BitXorExp)*
ExprPtr Parser::parseBitOrExpr() {
    auto lhs = parseBitXorExpr();
    while (check(TK::BitOr)) {
        int ln = peek().line;
        next();
        auto rhs = parseBitXorExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = "|";
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// BitXorExp → BitAndExp ('^' BitAndExp)*
ExprPtr Parser::parseBitXorExpr() {
    auto lhs = parseBitAndExpr();
    while (check(TK::BitXor)) {
        int ln = peek().line;
        next();
        auto rhs = parseBitAndExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = "^";
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// BitAndExp → EqExp ('&' EqExp)*
ExprPtr Parser::parseBitAndExpr() {
    auto lhs = parseEqExpr();
    while (check(TK::BitAnd)) {
        int ln = peek().line;
        next();
        auto rhs = parseEqExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = "&";
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// EqExp → RelExp (('==' | '!=') RelExp)*
ExprPtr Parser::parseEqExpr() {
    auto lhs = parseRelExpr();
    while (check(TK::Eq) || check(TK::Ne)) {
        int ln = peek().line;
        std::string op = next().text;
        auto rhs = parseRelExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = op;
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// RelExp → AddExp (('<' | '<=' | '>' | '>=') AddExp)*
ExprPtr Parser::parseRelExpr() {
    auto lhs = parseAddExpr();
    while (check(TK::Lt) || check(TK::Le) || check(TK::Gt) || check(TK::Ge)) {
        int ln = peek().line;
        std::string op = next().text;
        auto rhs = parseAddExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = op;
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// AddExp → MulExp (('+' | '-') MulExp)*
ExprPtr Parser::parseAddExpr() {
    auto lhs = parseMulExpr();
    while (check(TK::Plus) || check(TK::Minus)) {
        int ln = peek().line;
        std::string op = next().text;
        auto rhs = parseMulExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = op;
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// MulExp → UnaryExp (('*' | '/' | '%') UnaryExp)*
ExprPtr Parser::parseMulExpr() {
    auto lhs = parseUnaryExpr();
    while (check(TK::Star) || check(TK::Slash) || check(TK::Percent)) {
        int ln = peek().line;
        std::string op = next().text;
        auto rhs = parseUnaryExpr();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = ln; bin->op = op;
        bin->lhs = std::move(lhs); bin->rhs = std::move(rhs);
        lhs = std::move(bin);
    }
    return lhs;
}

// UnaryExp → ('+' | '-' | '!' | '~') UnaryExp | PrimaryExp
ExprPtr Parser::parseUnaryExpr() {
    if (check(TK::Plus) || check(TK::Minus) ||
        check(TK::LNot) || check(TK::BitNot)) {
        int ln = peek().line;
        char op = next().text[0];
        auto u = std::make_unique<UnaryExpr>();
        u->line = ln; u->op = op;
        u->operand = parseUnaryExpr();
        return u;
    }
    return parsePrimaryExpr();
}

// PrimaryExpr → '(' Exp ')' | Ident ['(' FuncRParams ')' | indices] | Number
ExprPtr Parser::parsePrimaryExpr() {
    int ln = peek().line;

    // 括号表达式
    if (check(TK::LParen)) {
        next();
        if (check(TK::Int) || check(TK::Char)) {
            auto cast = std::make_unique<CastExpr>();
            cast->line = ln;
            cast->targetType.base = parseBaseType();
            expect(TK::RParen);
            cast->operand = parseUnaryExpr();
            return cast;
        }
        // Parenthesized conditions are also integer-valued expressions in the
        // existing SysY2022 corpus (for example, (a > b && c > d) + 1).
        // Parse the complete conditional precedence chain here; if no
        // relation/logical operator is present this is identical to Exp.
        auto e = parseCond();
        expect(TK::RParen);
        return e;
    }

    // 整型字面量
    if (check(TK::IntLit)) {
        auto tok = next();
        auto e = std::make_unique<IntLitExpr>();
        e->line = ln; e->val = tok.ival;
        return e;
    }

    // 浮点字面量（2025年新增）
    if (check(TK::FloatLit)) {
        auto tok = next();
        auto e = std::make_unique<FloatLitExpr>();
        e->line = ln; e->val = tok.fval;
        return e;
    }

    if (check(TK::CharLit)) {
        auto tok = next();
        auto e = std::make_unique<CharLitExpr>();
        e->line = ln;
        e->val = static_cast<uint8_t>(tok.ival);
        return e;
    }

    // 字符串表达式：相邻字面量在语法层合并，再解析常量下标。
    if (check(TK::StringLit)) {
        auto e = std::make_unique<StringLitExpr>();
        e->line = ln;
        e->value = next().text;
        while (check(TK::StringLit)) e->value += next().text;
        while (check(TK::LBracket)) {
            next();
            e->indices.push_back(parseExpr());
            expect(TK::RBracket);
        }
        return e;
    }

    // 标识符：函数调用 或 左值
    if (check(TK::Id)) {
        std::string name = next().text;

        // 函数调用：Ident '(' ... ')'
        if (check(TK::LParen)) {
            next();
            auto call = std::make_unique<CallExpr>();
            call->line = ln; call->name = name;
            if (!check(TK::RParen)) {
                call->args.push_back(parseExpr());
                while (tryConsume(TK::Comma)) {
                    call->args.push_back(parseExpr());
                }
            }
            expect(TK::RParen);
            return call;
        }

        // 左值：Ident ('[' Exp ']')*
        auto lval = std::make_unique<LValExpr>();
        lval->line = ln; lval->name = name;
        while (check(TK::LBracket)) {
            next();
            lval->indices.push_back(parseExpr());
            expect(TK::RBracket);
        }
        return lval;
    }

    error("unexpected token '" + peek().text + "' in expression");
    return nullptr;
}
