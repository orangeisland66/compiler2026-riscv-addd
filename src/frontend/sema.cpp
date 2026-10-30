// 文件职责：执行 SysY2026 的作用域、类型和控制流语义检查。
#include "sema.h"

#include <algorithm>
#include <unordered_set>

namespace {

SysType scalar(BaseType base) {
    SysType type;
    type.base = base;
    return type;
}

bool isIntegerBase(BaseType base) {
    return base == BaseType::Int || base == BaseType::Char;
}

bool evalConstIndex(Expr* expr, int& value) {
    if (auto* lit = dynamic_cast<IntLitExpr*>(expr)) {
        value = lit->val;
        return true;
    }
    if (auto* ch = dynamic_cast<CharLitExpr*>(expr)) {
        value = ch->val;
        return true;
    }
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
        int v = 0;
        if (!evalConstIndex(unary->operand.get(), v)) return false;
        if (unary->op == '+') value = v;
        else if (unary->op == '-') value = -v;
        else if (unary->op == '~') value = ~v;
        else if (unary->op == '!') value = !v;
        else return false;
        return true;
    }
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        int lhs = 0, rhs = 0;
        if (!evalConstIndex(binary->lhs.get(), lhs) ||
            !evalConstIndex(binary->rhs.get(), rhs)) return false;
        if (binary->op == "+") value = lhs + rhs;
        else if (binary->op == "-") value = lhs - rhs;
        else if (binary->op == "*") value = lhs * rhs;
        else if (binary->op == "/" && rhs != 0) value = lhs / rhs;
        else if (binary->op == "%" && rhs != 0) value = lhs % rhs;
        else return false;
        return true;
    }
    return false;
}

} // namespace

void Sema::error(int line, char code) {
    diagnostics_.push_back({line, code});
}

void Sema::pushScope() { scopes_.push_back({}); }

void Sema::popScope() {
    if (!scopes_.empty()) scopes_.pop_back();
}

const Sema::SymbolInfo* Sema::lookup(const std::string& name) const {
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; --i) {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end()) return &it->second;
    }
    auto it = globals_.find(name);
    return it == globals_.end() ? nullptr : &it->second;
}

bool Sema::declare(const std::string& name, SymbolInfo info, int line,
                  bool global) {
    auto* table = global ? &globals_ : &scopes_.back();
    if (table->count(name)) {
        error(line, 'b');
        return false;
    }
    (*table)[name] = std::move(info);
    return true;
}

const Sema::FunctionInfo* Sema::lookupFunction(const std::string& name) const {
    auto it = functions_.find(name);
    return it == functions_.end() ? nullptr : &it->second;
}

bool Sema::isScalar(const SysType& type) const {
    return type.dims.empty() &&
           (type.base == BaseType::Int || type.base == BaseType::Char ||
            type.base == BaseType::Float);
}

bool Sema::sameType(const SysType& lhs, const SysType& rhs) const {
    return lhs.base == rhs.base && lhs.dims == rhs.dims;
}

bool Sema::compatibleValue(const SysType& actual, const SysType& expected) const {
    if (sameType(actual, expected)) return true;
    if (actual.isArray() || expected.isArray()) return false;
    // C integer promotion: char participates in scalar expressions and can
    // be passed/returned where int is expected.  The existing int/float
    // conversion contract remains unchanged.
    if ((actual.base == BaseType::Char && expected.base == BaseType::Int) ||
        (actual.base == BaseType::Int && expected.base == BaseType::Char))
        return true;
    return (actual.base == BaseType::Int && expected.base == BaseType::Float) ||
           (actual.base == BaseType::Float && expected.base == BaseType::Int) ||
           (actual.base == BaseType::Char && expected.base == BaseType::Float) ||
           (actual.base == BaseType::Float && expected.base == BaseType::Char);
}

bool Sema::compatibleParam(const SysType& actual, const SysType& expected) const {
    if (expected.isArray()) {
        if (!actual.isArray() || actual.base != expected.base) return false;
        // A one-dimensional formal array parameter is a pointer to its first
        // element; a multidimensional actual array may be passed to it.
        if (expected.dims.size() == 1 && expected.dims[0] < 0) return true;
        return actual.dims.size() == expected.dims.size();
    }
    return compatibleValue(actual, expected);
}

void Sema::collectTopLevel(CompUnit* unit) {
    for (auto& item : unit->items) {
        if (item.kind == CompUnit::Item::Kind::GlobalDecl) {
            for (auto& def : item.decl->defs) {
                if (globals_.count(def->name) || functions_.count(def->name))
                    error(def->line, 'b');
                else {
                    SymbolInfo info{def->type, def->isConst};
                    globals_[def->name] = std::move(info);
                }
            }
        } else {
            auto* func = item.func.get();
            if (globals_.count(func->name) || functions_.count(func->name)) {
                error(func->line, 'b');
                continue;
            }
            FunctionInfo info;
            info.retType = func->retType;
            for (auto& param : func->params) info.params.push_back(param->type);
            functions_[func->name] = std::move(info);
        }
    }
}

void Sema::checkGlobalDecl(DeclStmt* decl) {
    for (auto& def : decl->defs) {
        if (def->isConst && !def->initVal) error(def->line, 'e');
        if (def->initVal)
            checkInit(def->initVal.get(), def->type, true, def->isConst);
    }
}

void Sema::checkFunction(FuncDef* func) {
    currentReturn_ = func->retType;
    loopDepth_ = 0;
    switchDepth_ = 0;
    pushScope();
    for (auto& param : func->params)
        declare(param->name, {param->type, false}, param->line, false);
    checkBlock(func->body.get());
    popScope();
}

void Sema::checkBlock(BlockStmt* block) {
    pushScope();
    for (auto& item : block->items) {
        if (item.kind == BlockStmt::Item::Kind::Decl)
            checkDecl(item.decl.get());
        else if (item.stmt)
            checkStmt(item.stmt.get());
    }
    popScope();
}

void Sema::checkDecl(DeclStmt* decl) {
    for (auto& def : decl->defs) {
        declare(def->name, {def->type, def->isConst}, def->line, false);
        if (def->isConst && !def->initVal) error(def->line, 'e');
        if (def->initVal)
            checkInit(def->initVal.get(), def->type, false, def->isConst);
    }
}

void Sema::checkInit(InitVal* init, const SysType& target, bool global,
                     bool isConst) {
    if (auto* str = dynamic_cast<StringInit*>(init)) {
        if (!target.isArray() || target.base != BaseType::Char) error(init->line, 'e');
        if (target.isArray() && target.dims[0] >= 0 &&
            static_cast<int>(str->value.size()) > target.dims[0])
            error(init->line, 'e');
        return;
    }
    if (auto* agg = dynamic_cast<AggregateInit*>(init)) {
        SysType element = scalar(target.base);
        for (auto& item : agg->items)
            checkInit(item.get(), element, global, isConst);
        return;
    }
    auto* scalarInit = dynamic_cast<ScalarInit*>(init);
    if (!scalarInit) return;
    SysType actual = checkExpr(scalarInit->expr.get());
    if (!compatibleValue(actual, target)) error(init->line, 'e');
    if (global && !isConstExpr(scalarInit->expr.get())) error(init->line, 'e');
}

SysType Sema::checkLVal(LValExpr* lv, bool write) {
    const auto* symbol = lookup(lv->name);
    if (!symbol) {
        error(lv->line, 'c');
        return scalar(BaseType::Int);
    }
    if (write && symbol->isConst) error(lv->line, 'h');
    if (lv->indices.size() > symbol->type.dims.size()) {
        error(lv->line, 'k');
        return scalar(symbol->type.base);
    }
    for (auto& index : lv->indices) {
        SysType indexType = checkExpr(index.get());
        if (!isIntegerBase(indexType.base) || indexType.isArray())
            error(index->line, 'e');
    }
    SysType result = symbol->type;
    result.dims.erase(result.dims.begin(),
                      result.dims.begin() + static_cast<long>(lv->indices.size()));
    lv->semaType = result;
    return result;
}

SysType Sema::checkExpr(Expr* expr, bool condition) {
    if (!expr) return scalar(BaseType::Int);
    if (dynamic_cast<IntLitExpr*>(expr)) {
        expr->semaType = scalar(BaseType::Int);
        return *expr->semaType;
    }
    if (dynamic_cast<FloatLitExpr*>(expr)) {
        expr->semaType = scalar(BaseType::Float);
        return *expr->semaType;
    }
    if (dynamic_cast<CharLitExpr*>(expr)) {
        expr->semaType = scalar(BaseType::Char);
        return *expr->semaType;
    }
    if (auto* str = dynamic_cast<StringLitExpr*>(expr)) {
        if (str->indices.size() != 1) {
            error(str->line, 'e');
            expr->semaType = scalar(BaseType::Char);
            return *expr->semaType;
        }
        SysType indexType = checkExpr(str->indices.front().get());
        if (!isIntegerBase(indexType.base) || indexType.isArray())
            error(str->indices.front()->line, 'e');
        int index = 0;
        if (!evalConstIndex(str->indices.front().get(), index) ||
            index < 0 || index >= static_cast<int>(str->value.size()))
            error(str->indices.front()->line, 'e');
        expr->semaType = scalar(BaseType::Char);
        return *expr->semaType;
    }
    if (auto* lv = dynamic_cast<LValExpr*>(expr)) return checkLVal(lv, false);
    if (auto* cast = dynamic_cast<CastExpr*>(expr)) {
        SysType source = checkExpr(cast->operand.get());
        if (!isScalar(source) ||
            (cast->targetType.base != BaseType::Int &&
             cast->targetType.base != BaseType::Char))
            error(cast->line, 'e');
        expr->semaType = cast->targetType;
        return *expr->semaType;
    }
    if (auto* call = dynamic_cast<CallExpr*>(expr)) {
        const auto* fn = lookupFunction(call->name);
        if (!fn) {
            // SysY2022 runtime aliases remain recognized only for compatibility.
            static const std::unordered_set<std::string> builtins = {
                "getarray", "putarray", "getfloat", "putfloat",
                "starttime", "stoptime"};
            if (!builtins.count(call->name)) error(call->line, 'c');
            for (auto& arg : call->args) checkExpr(arg.get());
            expr->semaType = scalar(BaseType::Int);
            return *expr->semaType;
        }
        if (call->args.size() != fn->params.size()) error(call->line, 'd');
        for (size_t i = 0; i < call->args.size(); ++i) {
            SysType actual = checkExpr(call->args[i].get());
            if (i < fn->params.size() && !compatibleParam(actual, fn->params[i]))
                error(call->args[i]->line, 'e');
        }
        expr->semaType = fn->retType;
        return *expr->semaType;
    }
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
        SysType operand = checkExpr(unary->operand.get(), condition);
        if (!isScalar(operand)) error(unary->line, 'e');
        if (unary->op == '~' && operand.base == BaseType::Float)
            error(unary->line, 'e');
        expr->semaType = (unary->op == '!' || unary->op == '~' ||
                          ((unary->op == '+' || unary->op == '-') &&
                           operand.base == BaseType::Char))
                             ? scalar(BaseType::Int)
                             : operand;
        return *expr->semaType;
    }
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        const bool logical = binary->op == "&&" || binary->op == "||";
        const bool bitwise = binary->op == "&" || binary->op == "|" ||
                             binary->op == "^";
        SysType lhs = checkExpr(binary->lhs.get(), condition || logical);
        SysType rhs = checkExpr(binary->rhs.get(), condition || logical);
        if (logical) {
            if (!isScalar(lhs) || !isScalar(rhs)) error(binary->line, 'e');
            expr->semaType = scalar(BaseType::Int);
            return *expr->semaType;
        }
        if (bitwise) {
            if (!isScalar(lhs) || !isScalar(rhs) ||
                lhs.base == BaseType::Float || rhs.base == BaseType::Float)
                error(binary->line, 'e');
            expr->semaType = scalar(BaseType::Int);
            return *expr->semaType;
        }
        if (!compatibleValue(lhs, rhs) || !isScalar(lhs)) error(binary->line, 'e');
        if (binary->op == "==" || binary->op == "!=" || binary->op == "<" ||
            binary->op == "<=" || binary->op == ">" || binary->op == ">=")
            expr->semaType = scalar(BaseType::Int);
        else if (lhs.base == BaseType::Float || rhs.base == BaseType::Float)
            expr->semaType = scalar(BaseType::Float);
        else
            expr->semaType = scalar(BaseType::Int);
        return *expr->semaType;
    }
    return scalar(BaseType::Int);
}

void Sema::checkStmt(Stmt* stmt) {
    if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
        auto* lv = dynamic_cast<LValExpr*>(assign->lval.get());
        if (!lv) error(assign->line, 'e');
        SysType lhs = lv ? checkLVal(lv, true) : scalar(BaseType::Int);
        SysType rhs = checkExpr(assign->rhs.get());
        if (!compatibleValue(rhs, lhs)) error(assign->line, 'e');
    } else if (auto* expr = dynamic_cast<ExprStmt*>(stmt)) {
        if (expr->expr) checkExpr(expr->expr.get());
    } else if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
        checkBlock(block);
    } else if (auto* cond = dynamic_cast<IfStmt*>(stmt)) {
        if (!isScalar(checkExpr(cond->cond.get(), true))) error(cond->line, 'e');
        checkStmt(cond->thenBranch.get());
        if (cond->elseBranch) checkStmt(cond->elseBranch.get());
    } else if (auto* loop = dynamic_cast<WhileStmt*>(stmt)) {
        if (!isScalar(checkExpr(loop->cond.get(), true))) error(loop->line, 'e');
        ++loopDepth_;
        checkStmt(loop->body.get());
        --loopDepth_;
    } else if (auto* loop = dynamic_cast<DoWhileStmt*>(stmt)) {
        ++loopDepth_;
        checkStmt(loop->body.get());
        if (!isScalar(checkExpr(loop->cond.get(), true))) error(loop->line, 'e');
        --loopDepth_;
    } else if (auto* sw = dynamic_cast<SwitchStmt*>(stmt)) {
        SysType value = checkExpr(sw->value.get());
        if (!isIntegerBase(value.base) || !value.dims.empty()) error(sw->line, 'e');
        ++switchDepth_;
        std::unordered_set<int> cases;
        bool hasDefault = false;
        for (auto& clause : sw->clauses) {
            if (clause.isDefault) {
                if (hasDefault) error(clause.line, 'n');
                hasDefault = true;
            } else {
                SysType caseType = checkExpr(clause.value.get());
                if (!sameType(caseType, value)) error(clause.line, 'e');
                int number = dynamic_cast<IntLitExpr*>(clause.value.get())
                                 ? dynamic_cast<IntLitExpr*>(clause.value.get())->val
                                 : dynamic_cast<CharLitExpr*>(clause.value.get())->val;
                if (!cases.insert(number).second) error(clause.line, 'n');
            }
            for (auto& body : clause.statements) checkStmt(body.get());
        }
        --switchDepth_;
    } else if (dynamic_cast<BreakStmt*>(stmt)) {
        if (loopDepth_ == 0 && switchDepth_ == 0) error(stmt->line, 'm');
    } else if (dynamic_cast<ContinueStmt*>(stmt)) {
        if (loopDepth_ == 0) error(stmt->line, 'm');
    } else if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
        if (currentReturn_.isVoid()) {
            if (ret->val) error(ret->line, 'f');
        } else {
            if (!ret->val ||
                !compatibleValue(checkExpr(ret->val.get()), currentReturn_))
                error(ret->line, 'e');
        }
    } else if (auto* printfStmt = dynamic_cast<PrintfStmt*>(stmt)) {
        size_t expected = 0;
        for (size_t i = 0; i + 1 < printfStmt->format.size(); ++i) {
            if (printfStmt->format[i] == '%') {
                ++expected;
                ++i;
            }
        }
        if (expected != printfStmt->args.size()) error(printfStmt->line, 'l');
        size_t arg = 0;
        for (size_t i = 0; i + 1 < printfStmt->format.size(); ++i) {
            if (printfStmt->format[i] != '%') continue;
            if (arg >= printfStmt->args.size()) break;
            SysType actual = checkExpr(printfStmt->args[arg++].get());
            BaseType expectedType = printfStmt->format[i + 1] == 'c' ? BaseType::Char :
                                    printfStmt->format[i + 1] == 's' ? BaseType::Char :
                                                                        BaseType::Int;
            if (printfStmt->format[i + 1] == 's') {
                if (!actual.isArray() || actual.base != BaseType::Char) error(printfStmt->line, 'e');
            } else if ((printfStmt->format[i + 1] == 'c' &&
                        (!isIntegerBase(actual.base) || actual.isArray())) ||
                       (printfStmt->format[i + 1] != 'c' &&
                        (actual.base != expectedType || actual.isArray()))) {
                error(printfStmt->line, 'e');
            }
            ++i;
        }
    }
}

bool Sema::isConstExpr(Expr* expr, BaseType* result) {
    if (dynamic_cast<IntLitExpr*>(expr)) { if (result) *result = BaseType::Int; return true; }
    if (dynamic_cast<FloatLitExpr*>(expr)) { if (result) *result = BaseType::Float; return true; }
    if (dynamic_cast<CharLitExpr*>(expr)) { if (result) *result = BaseType::Char; return true; }
    if (auto* lv = dynamic_cast<LValExpr*>(expr)) {
        const auto* symbol = lookup(lv->name);
        if (symbol && symbol->isConst && lv->indices.empty()) {
            if (result) *result = symbol->type.base;
            return true;
        }
        return false;
    }
    if (auto* cast = dynamic_cast<CastExpr*>(expr)) {
        BaseType source = BaseType::Void;
        if (!isConstExpr(cast->operand.get(), &source)) return false;
        if (result) *result = cast->targetType.base;
        return true;
    }
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr))
        return isConstExpr(unary->operand.get(), result);
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        BaseType lhs = BaseType::Void, rhs = BaseType::Void;
        if (!isConstExpr(binary->lhs.get(), &lhs) ||
            !isConstExpr(binary->rhs.get(), &rhs))
            return false;
        if (lhs != rhs && !((lhs == BaseType::Int && rhs == BaseType::Float) ||
                            (lhs == BaseType::Float && rhs == BaseType::Int)))
            return false;
        if (result)
            *result = (lhs == BaseType::Float || rhs == BaseType::Float)
                          ? BaseType::Float
                          : BaseType::Int;
        return true;
    }
    return false;
}

std::vector<SemaDiagnostic> Sema::analyze(CompUnit* unit) {
    diagnostics_.clear();
    scopes_.clear();
    globals_.clear();
    functions_.clear();
    auto addRuntime = [&](const std::string& name, BaseType ret,
                          std::vector<SysType> params = {}) {
        functions_.try_emplace(
            name, FunctionInfo{scalar(ret), std::move(params)});
    };
    collectTopLevel(unit);
    SysType intArray = scalar(BaseType::Int);
    intArray.dims.push_back(-1);
    SysType floatArray = scalar(BaseType::Float);
    floatArray.dims.push_back(-1);
    SysType charArray = scalar(BaseType::Char);
    charArray.dims.push_back(-1);
    addRuntime("getint", BaseType::Int);
    addRuntime("getch", BaseType::Int);
    addRuntime("getfloat", BaseType::Float);
    addRuntime("getarray", BaseType::Int, {intArray});
    addRuntime("getfarray", BaseType::Int, {floatArray});
    addRuntime("getstr", BaseType::Void, {charArray, scalar(BaseType::Int)});
    addRuntime("putint", BaseType::Void, {scalar(BaseType::Int)});
    addRuntime("putch", BaseType::Void, {scalar(BaseType::Int)});
    addRuntime("putfloat", BaseType::Void, {scalar(BaseType::Float)});
    addRuntime("putarray", BaseType::Void, {scalar(BaseType::Int), intArray});
    addRuntime("putfarray", BaseType::Void,
               {scalar(BaseType::Int), floatArray});
    addRuntime("putstr", BaseType::Void, {charArray});
    for (auto& item : unit->items) {
        if (item.kind == CompUnit::Item::Kind::GlobalDecl)
            checkGlobalDecl(item.decl.get());
        else
            checkFunction(item.func.get());
    }
    std::sort(diagnostics_.begin(), diagnostics_.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.line != rhs.line) return lhs.line < rhs.line;
        return lhs.code < rhs.code;
    });
    diagnostics_.erase(std::unique(diagnostics_.begin(), diagnostics_.end(),
        [](const auto& lhs, const auto& rhs) {
            return lhs.line == rhs.line && lhs.code == rhs.code;
        }), diagnostics_.end());
    return diagnostics_;
}
