// 文件职责：识别矩阵乘法中的循环结构，并整理连续列访问与寄存器累加过程。
// 变换前：for j { temp=0; for k { temp += A[i][k]*B[k][j]; } C[i][j]=temp; }
// 变换后：按列块维护多个累加值，减少每次迭代对 C[i][j] 的加载和存储。
#include "ast_matmul_interchange.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hir {
namespace {

using namespace std;

static constexpr int TILE = 2;

// === AST 辅助函数 ===

bool isScalarVar(const Expr* e, const string& name) {
    auto* lv = dynamic_cast<const LValExpr*>(e);
    return lv && lv->name == name && lv->indices.empty();
}

bool isZeroInit(const BlockStmt::Item& item, string& varName) {
    if (item.kind == BlockStmt::Item::Kind::Stmt) {
        auto* assign = dynamic_cast<AssignStmt*>(item.stmt.get());
        if (!assign) return false;
        auto* lv = dynamic_cast<LValExpr*>(assign->lval.get());
        if (!lv || !lv->indices.empty()) return false;
        auto* lit = dynamic_cast<IntLitExpr*>(assign->rhs.get());
        if (!lit || lit->val != 0) return false;
        varName = lv->name;
        return true;
    }
    if (item.kind == BlockStmt::Item::Kind::Decl) {
        auto* decl = item.decl.get();
        if (!decl || decl->defs.size() != 1) return false;
        auto* def = decl->defs[0].get();
        if (def->isConst) return false;
        auto* si = dynamic_cast<ScalarInit*>(def->initVal.get());
        if (!si) return false;
        auto* lit = dynamic_cast<IntLitExpr*>(si->expr.get());
        if (!lit || lit->val != 0) return false;
        varName = def->name;
        return true;
    }
    return false;
}

bool isIncrement(const Stmt* stmt, const string& varName, int step = 1) {
    auto* assign = dynamic_cast<const AssignStmt*>(stmt);
    if (!assign) return false;
    auto* lv = dynamic_cast<LValExpr*>(assign->lval.get());
    if (!lv || lv->name != varName || !lv->indices.empty()) return false;
    auto* bin = dynamic_cast<BinaryExpr*>(assign->rhs.get());
    if (!bin || bin->op != "+") return false;
    if (!isScalarVar(bin->lhs.get(), varName)) return false;
    auto* lit = dynamic_cast<IntLitExpr*>(bin->rhs.get());
    return lit && lit->val == step;
}

bool isLTCond(const Expr* cond, const string& varName, const Expr*& bound) {
    auto* bin = dynamic_cast<const BinaryExpr*>(cond);
    if (!bin || bin->op != "<") return false;
    if (!isScalarVar(bin->lhs.get(), varName)) return false;
    bound = bin->rhs.get();
    return true;
}

bool containsVar(const Expr* e, const string& name) {
    if (!e) return false;
    if (auto* lv = dynamic_cast<const LValExpr*>(e)) {
        if (lv->name == name) return true;
        for (auto& idx : lv->indices)
            if (containsVar(idx.get(), name)) return true;
        return false;
    }
    if (auto* bin = dynamic_cast<const BinaryExpr*>(e))
        return containsVar(bin->lhs.get(), name) || containsVar(bin->rhs.get(), name);
    if (auto* un = dynamic_cast<const UnaryExpr*>(e))
        return containsVar(un->operand.get(), name);
    if (auto* call = dynamic_cast<const CallExpr*>(e)) {
        for (auto& arg : call->args)
            if (containsVar(arg.get(), name)) return true;
    }
    return false;
}

// 深拷贝表达式。
ExprPtr cloneExpr(const Expr* e) {
    if (!e) return nullptr;
    if (auto* lit = dynamic_cast<const IntLitExpr*>(e)) {
        auto c = make_unique<IntLitExpr>(); c->val = lit->val; return c;
    }
    if (auto* lv = dynamic_cast<const LValExpr*>(e)) {
        auto c = make_unique<LValExpr>(); c->name = lv->name;
        for (auto& idx : lv->indices) c->indices.push_back(cloneExpr(idx.get()));
        return c;
    }
    if (auto* bin = dynamic_cast<const BinaryExpr*>(e)) {
        auto c = make_unique<BinaryExpr>(); c->op = bin->op;
        c->lhs = cloneExpr(bin->lhs.get()); c->rhs = cloneExpr(bin->rhs.get());
        return c;
    }
    if (auto* un = dynamic_cast<const UnaryExpr*>(e)) {
        auto c = make_unique<UnaryExpr>(); c->op = un->op;
        c->operand = cloneExpr(un->operand.get()); return c;
    }
    if (auto* call = dynamic_cast<const CallExpr*>(e)) {
        auto c = make_unique<CallExpr>(); c->name = call->name;
        for (auto& arg : call->args) c->args.push_back(cloneExpr(arg.get()));
        return c;
    }
    return nullptr;
}

// 深拷贝语句。
StmtPtr cloneStmt(const Stmt* s) {
    if (!s) return nullptr;
    if (auto* assign = dynamic_cast<const AssignStmt*>(s)) {
        auto c = make_unique<AssignStmt>();
        c->lval = cloneExpr(assign->lval.get());
        c->rhs = cloneExpr(assign->rhs.get());
        return c;
    }
    if (auto* block = dynamic_cast<const BlockStmt*>(s)) {
        auto c = make_unique<BlockStmt>();
        for (auto& item : block->items) {
            BlockStmt::Item ni; ni.kind = item.kind;
            if (item.kind == BlockStmt::Item::Kind::Stmt)
                ni.stmt = cloneStmt(item.stmt.get());
            c->items.push_back(move(ni));
        }
        return c;
    }
    if (auto* whileS = dynamic_cast<const WhileStmt*>(s)) {
        auto c = make_unique<WhileStmt>();
        c->cond = cloneExpr(whileS->cond.get());
        c->body = cloneStmt(whileS->body.get());
        return c;
    }
    if (auto* ifS = dynamic_cast<const IfStmt*>(s)) {
        auto c = make_unique<IfStmt>();
        c->cond = cloneExpr(ifS->cond.get());
        c->thenBranch = cloneStmt(ifS->thenBranch.get());
        c->elseBranch = cloneStmt(ifS->elseBranch.get());
        return c;
    }
    return nullptr;
}

// 在表达式中替换标量变量名。
void replaceVar(Expr* e, const string& from, const string& to) {
    if (!e) return;
    if (auto* lv = dynamic_cast<LValExpr*>(e)) {
        if (lv->name == from && lv->indices.empty()) { lv->name = to; return; }
        for (auto& idx : lv->indices) replaceVar(idx.get(), from, to);
    } else if (auto* bin = dynamic_cast<BinaryExpr*>(e)) {
        replaceVar(bin->lhs.get(), from, to);
        replaceVar(bin->rhs.get(), from, to);
    } else if (auto* un = dynamic_cast<UnaryExpr*>(e)) {
        replaceVar(un->operand.get(), from, to);
    }
}

void replaceVarInStmt(Stmt* s, const string& from, const string& to) {
    if (!s) return;
    if (auto* assign = dynamic_cast<AssignStmt*>(s)) {
        replaceVar(assign->lval.get(), from, to);
        replaceVar(assign->rhs.get(), from, to);
    } else if (auto* block = dynamic_cast<BlockStmt*>(s)) {
        for (auto& item : block->items)
            if (item.kind == BlockStmt::Item::Kind::Stmt)
                replaceVarInStmt(item.stmt.get(), from, to);
    } else if (auto* whileS = dynamic_cast<WhileStmt*>(s)) {
        replaceVar(whileS->cond.get(), from, to);
        replaceVarInStmt(whileS->body.get(), from, to);
    } else if (auto* ifS = dynamic_cast<IfStmt*>(s)) {
        replaceVar(ifS->cond.get(), from, to);
        replaceVarInStmt(ifS->thenBranch.get(), from, to);
        replaceVarInStmt(ifS->elseBranch.get(), from, to);
    }
}

// 在语句中替换累加器变量名。
void replaceAccInStmt(Stmt* s, const string& accVar, const string& newAcc) {
    if (!s) return;
    if (auto* assign = dynamic_cast<AssignStmt*>(s)) {
        if (isScalarVar(assign->lval.get(), accVar)) {
            auto* lv = dynamic_cast<LValExpr*>(assign->lval.get());
            lv->name = newAcc;
        }
        replaceVar(assign->rhs.get(), accVar, newAcc);
    } else if (auto* ifS = dynamic_cast<IfStmt*>(s)) {
        replaceVar(ifS->cond.get(), accVar, newAcc);
        replaceAccInStmt(ifS->thenBranch.get(), accVar, newAcc);
        replaceAccInStmt(ifS->elseBranch.get(), accVar, newAcc);
    } else if (auto* block = dynamic_cast<BlockStmt*>(s)) {
        for (auto& item : block->items)
            if (item.kind == BlockStmt::Item::Kind::Stmt)
                replaceAccInStmt(item.stmt.get(), accVar, newAcc);
    }
}

// 构造 varName = varName + step。
StmtPtr makeIncrement(const string& varName, int step) {
    auto assign = make_unique<AssignStmt>();
    auto lv = make_unique<LValExpr>(); lv->name = varName;
    assign->lval = move(lv);
    auto bin = make_unique<BinaryExpr>(); bin->op = "+";
    auto lhs = make_unique<LValExpr>(); lhs->name = varName;
    bin->lhs = move(lhs);
    auto rhs = make_unique<IntLitExpr>(); rhs->val = step;
    bin->rhs = move(rhs);
    assign->rhs = move(bin);
    return assign;
}

// 构造 arr[idx0][idx1] = varName。
StmtPtr makeArrayStore(const string& arr, const string& idx0, ExprPtr idx1, const string& varName) {
    auto assign = make_unique<AssignStmt>();
    auto lv = make_unique<LValExpr>(); lv->name = arr;
    auto i0 = make_unique<LValExpr>(); i0->name = idx0;
    lv->indices.push_back(move(i0));
    lv->indices.push_back(move(idx1));
    assign->lval = move(lv);
    auto rv = make_unique<LValExpr>(); rv->name = varName;
    assign->rhs = move(rv);
    return assign;
}

// 构造 varName + offset 表达式。
ExprPtr makeVarPlusInt(const string& varName, int offset) {
    if (offset == 0) {
        auto lv = make_unique<LValExpr>(); lv->name = varName;
        return lv;
    }
    auto bin = make_unique<BinaryExpr>(); bin->op = "+";
    auto lv = make_unique<LValExpr>(); lv->name = varName;
    bin->lhs = move(lv);
    auto lit = make_unique<IntLitExpr>(); lit->val = offset;
    bin->rhs = move(lit);
    return bin;
}

// === 主要变换 ===

// 尝试在 i 循环体内的 j 循环上应用列块整理。
bool tryInterchange(BlockStmt* outerBody, const string& rowVar) {
    // 形状：outerBody 包含 [j_init, j_loop, i_inc] 三个元素。
    if (outerBody->items.size() != 3) return false;

    string colVar;
    if (!isZeroInit(outerBody->items[0], colVar)) return false;

    if (outerBody->items[1].kind != BlockStmt::Item::Kind::Stmt) return false;
    auto* jLoop = dynamic_cast<WhileStmt*>(outerBody->items[1].stmt.get());
    if (!jLoop) return false;
    const Expr* jBound = nullptr;
    if (!isLTCond(jLoop->cond.get(), colVar, jBound)) return false;

    // j 循环体：{ k=0; temp=0; while(k<N) { ... k++; } C[i][j]=temp; j++; }。
    auto* jBody = dynamic_cast<BlockStmt*>(jLoop->body.get());
    if (!jBody || jBody->items.size() != 5) return false;

    string kVar;
    if (!isZeroInit(jBody->items[0], kVar)) return false;
    string accVar;
    if (!isZeroInit(jBody->items[1], accVar)) return false;

    if (jBody->items[2].kind != BlockStmt::Item::Kind::Stmt) return false;
    auto* kLoop = dynamic_cast<WhileStmt*>(jBody->items[2].stmt.get());
    if (!kLoop) return false;
    const Expr* kBound = nullptr;
    if (!isLTCond(kLoop->cond.get(), kVar, kBound)) return false;

    auto* kBody = dynamic_cast<BlockStmt*>(kLoop->body.get());
    if (!kBody || kBody->items.size() < 2) return false;

    // 最后一个元素是 k++。
    if (kBody->items.back().kind != BlockStmt::Item::Kind::Stmt) return false;
    if (!isIncrement(kBody->items.back().stmt.get(), kVar)) return false;

    // 检查累加形状：accVar 出现在加法赋值中。
    bool foundAcc = false;
    for (size_t si = 0; si + 1 < kBody->items.size(); ++si) {
        if (kBody->items[si].kind != BlockStmt::Item::Kind::Stmt) continue;
        auto* stmt = kBody->items[si].stmt.get();
        if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
            if (isScalarVar(assign->lval.get(), accVar) && containsVar(assign->rhs.get(), accVar))
                foundAcc = true;
        }
        if (auto* ifS = dynamic_cast<IfStmt*>(stmt)) {
            auto checkThen = [&](Stmt* then) {
                if (auto* ta = dynamic_cast<AssignStmt*>(then)) {
                    if (isScalarVar(ta->lval.get(), accVar) && containsVar(ta->rhs.get(), accVar))
                        foundAcc = true;
                }
                if (auto* tb = dynamic_cast<BlockStmt*>(then)) {
                    for (auto& ti : tb->items)
                        if (ti.kind == BlockStmt::Item::Kind::Stmt)
                            if (auto* ta = dynamic_cast<AssignStmt*>(ti.stmt.get()))
                                if (isScalarVar(ta->lval.get(), accVar) && containsVar(ta->rhs.get(), accVar))
                                    foundAcc = true;
                }
            };
            checkThen(ifS->thenBranch.get());
        }
    }
    if (!foundAcc) return false;

    // k 循环结束后写回 C[i][j] = temp。
    if (jBody->items[3].kind != BlockStmt::Item::Kind::Stmt) return false;
    auto* storeStmt = dynamic_cast<AssignStmt*>(jBody->items[3].stmt.get());
    if (!storeStmt) return false;
    auto* storeLv = dynamic_cast<LValExpr*>(storeStmt->lval.get());
    if (!storeLv || storeLv->indices.size() != 2) return false;
    string dstArr = storeLv->name;
    auto* dstI0 = dynamic_cast<LValExpr*>(storeLv->indices[0].get());
    auto* dstI1 = dynamic_cast<LValExpr*>(storeLv->indices[1].get());
    if (!dstI0 || !dstI1 || dstI0->name != rowVar || dstI1->name != colVar) return false;
    if (!isScalarVar(storeStmt->rhs.get(), accVar)) return false;

    // 最后一个元素是 j++。
    if (jBody->items[4].kind != BlockStmt::Item::Kind::Stmt) return false;
    if (!isIncrement(jBody->items[4].stmt.get(), colVar)) return false;

    // 目标数组不能在 k 循环体中再次使用，避免产生别名访问。
    bool dstUsed = false;
    for (size_t si = 0; si + 1 < kBody->items.size(); ++si) {
        if (kBody->items[si].kind != BlockStmt::Item::Kind::Stmt) continue;
        auto* stmt = kBody->items[si].stmt.get();
        function<void(const Expr*)> checkDst = [&](const Expr* e) {
            if (!e) return;
            if (auto* lv = dynamic_cast<const LValExpr*>(e)) {
                if (lv->name == dstArr && !lv->indices.empty()) dstUsed = true;
                for (auto& idx : lv->indices) checkDst(idx.get());
            } else if (auto* bin = dynamic_cast<const BinaryExpr*>(e)) {
                checkDst(bin->lhs.get()); checkDst(bin->rhs.get());
            } else if (auto* un = dynamic_cast<const UnaryExpr*>(e)) {
                checkDst(un->operand.get());
            }
        };
        if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) checkDst(assign->rhs.get());
        if (auto* ifS = dynamic_cast<IfStmt*>(stmt)) {
            checkDst(ifS->cond.get());
            if (auto* ta = dynamic_cast<AssignStmt*>(ifS->thenBranch.get())) checkDst(ta->rhs.get());
            if (auto* tb = dynamic_cast<BlockStmt*>(ifS->thenBranch.get()))
                for (auto& ti : tb->items)
                    if (ti.kind == BlockStmt::Item::Kind::Stmt)
                        if (auto* ta = dynamic_cast<AssignStmt*>(ti.stmt.get())) checkDst(ta->rhs.get());
        }
    }
    if (dstUsed) return false;

    // === 生成变换后的代码 ===
    // 结构如下：
    //   int j2 = 0;
    //   while (j2 + (TILE-1) < bound) {   // 主循环
    //     int acc0=0, acc1=0, acc2=0, acc3=0;
    //     int k = 0;
    //     while (k < kBound) {
    //       循环体，其中 j 替换为 j2、acc 替换为 acc0
    //       循环体，其中 j 替换为 j2+1、acc 替换为 acc1
    //       循环体，其中 j 替换为 j2+2、acc 替换为 acc2
    //       循环体，其中 j 替换为 j2+3、acc 替换为 acc3
    //       k++;
    //     }
    //     C[i][j2]=acc0; C[i][j2+1]=acc1; C[i][j2+2]=acc2; C[i][j2+3]=acc3;
    //     j2 += TILE;
    //   }
    //   while (j2 < bound) {   // 处理剩余列
    //     int temp = 0; int k = 0;
    //     while (k < kBound) { <original body with j->j2> k++; }
    //     C[i][j2] = temp;
    //     j2++;
    //   }

    string j2Var = colVar + "_t";
    string accNames[TILE];
    for (int t = 0; t < TILE; ++t) accNames[t] = accVar + "_r" + to_string(t);

    auto newOuterBody = make_unique<BlockStmt>();

    // 保留原始 j 的声明和初始化。列块循环内部使用 j2，但外层函数或最后的
    // 同步代码仍可能引用原始 AST 变量；删除第一个元素会造成重写后的标识符未定义。
    newOuterBody->items.push_back(move(outerBody->items[0]));

    // int j2 = 0;
    {
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = j2Var; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        auto z = make_unique<IntLitExpr>(); z->val = 0;
        si->expr = move(z); def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item di; di.kind = BlockStmt::Item::Kind::Decl; di.decl = move(decl);
        newOuterBody->items.push_back(move(di));
    }

    // 主列块循环：while (j2 + TILE-1 < bound)。
    auto mainLoop = make_unique<WhileStmt>();
    {
        auto cond = make_unique<BinaryExpr>(); cond->op = "<";
        auto lhs = make_unique<BinaryExpr>(); lhs->op = "+";
        auto j2lv = make_unique<LValExpr>(); j2lv->name = j2Var;
        lhs->lhs = move(j2lv);
        auto tileLit = make_unique<IntLitExpr>(); tileLit->val = TILE - 1;
        lhs->rhs = move(tileLit);
        cond->lhs = move(lhs);
        cond->rhs = cloneExpr(jBound);
        mainLoop->cond = move(cond);
    }

    auto mainBody = make_unique<BlockStmt>();

    // 声明累加器：int acc0=0, acc1=0, acc2=0, acc3=0。
    for (int t = 0; t < TILE; ++t) {
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = accNames[t]; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        auto z = make_unique<IntLitExpr>(); z->val = 0;
        si->expr = move(z); def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item di; di.kind = BlockStmt::Item::Kind::Decl; di.decl = move(decl);
        mainBody->items.push_back(move(di));
    }

    // int k = 0;
    {
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = kVar; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        auto z = make_unique<IntLitExpr>(); z->val = 0;
        si->expr = move(z); def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item di; di.kind = BlockStmt::Item::Kind::Decl; di.decl = move(decl);
        mainBody->items.push_back(move(di));
    }

    // 在 k 循环前创建 jt0 到 jt3 临时变量。
    string jtVars[TILE];
    for (int t = 0; t < TILE; ++t) {
        jtVars[t] = colVar + "_j" + to_string(t);
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = jtVars[t]; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        si->expr = makeVarPlusInt(j2Var, t);
        def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item di; di.kind = BlockStmt::Item::Kind::Decl; di.decl = move(decl);
        mainBody->items.push_back(move(di));
    }

    // 内层 k 循环复制 TILE 份循环体。
    auto newKLoop = make_unique<WhileStmt>();
    newKLoop->cond = cloneExpr(kLoop->cond.get());

    auto newKBody = make_unique<BlockStmt>();

    // 为每个列块元素复制一份 k 循环体。
    for (int t = 0; t < TILE; ++t) {
        for (size_t si = 0; si + 1 < kBody->items.size(); ++si) {
            if (kBody->items[si].kind != BlockStmt::Item::Kind::Stmt) continue;
            auto cloned = cloneStmt(kBody->items[si].stmt.get());
            if (!cloned) continue;
            // 将列变量 colVar（j）替换为 jtVars[t]（j2+t）。
            replaceVarInStmt(cloned.get(), colVar, jtVars[t]);
            // 将累加器变量 accVar 替换为 accNames[t]。
            replaceAccInStmt(cloned.get(), accVar, accNames[t]);
            BlockStmt::Item ci; ci.kind = BlockStmt::Item::Kind::Stmt; ci.stmt = move(cloned);
            newKBody->items.push_back(move(ci));
        }
    }

    // k++ at end of k-loop body
    {
        BlockStmt::Item ki; ki.kind = BlockStmt::Item::Kind::Stmt;
        ki.stmt = makeIncrement(kVar, 1);
        newKBody->items.push_back(move(ki));
    }

    newKLoop->body = move(newKBody);
    {
        BlockStmt::Item ki; ki.kind = BlockStmt::Item::Kind::Stmt;
        ki.stmt = move(newKLoop);
        mainBody->items.push_back(move(ki));
    }

    // 写回：C[i][j2]=acc0; C[i][j2+1]=acc1; …。
    for (int t = 0; t < TILE; ++t) {
        BlockStmt::Item si; si.kind = BlockStmt::Item::Kind::Stmt;
        si.stmt = makeArrayStore(dstArr, rowVar, makeVarPlusInt(j2Var, t), accNames[t]);
        mainBody->items.push_back(move(si));
    }

    // j2 += TILE。
    {
        BlockStmt::Item ji; ji.kind = BlockStmt::Item::Kind::Stmt;
        ji.stmt = makeIncrement(j2Var, TILE);
        mainBody->items.push_back(move(ji));
    }

    mainLoop->body = move(mainBody);
    {
        BlockStmt::Item li; li.kind = BlockStmt::Item::Kind::Stmt;
        li.stmt = move(mainLoop);
        newOuterBody->items.push_back(move(li));
    }

    // 剩余循环：while (j2 < bound)，使用 j 替换为 j2 后的原始循环体。
    auto remLoop = make_unique<WhileStmt>();
    {
        auto cond = make_unique<BinaryExpr>(); cond->op = "<";
        auto j2lv = make_unique<LValExpr>(); j2lv->name = j2Var;
        cond->lhs = move(j2lv);
        cond->rhs = cloneExpr(jBound);
        remLoop->cond = move(cond);
    }
    auto remBody = make_unique<BlockStmt>();
    // 声明 int temp = 0。
    {
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = accVar; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        auto z = make_unique<IntLitExpr>(); z->val = 0;
        si->expr = move(z); def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item ti; ti.kind = BlockStmt::Item::Kind::Decl; ti.decl = move(decl);
        remBody->items.push_back(move(ti));
    }
    // int k = 0; (declaration)
    {
        auto decl = make_unique<DeclStmt>();
        decl->isConst = false; decl->baseType = BaseType::Int;
        auto def = make_unique<VarDef>();
        def->name = kVar; def->isConst = false; def->type = SysType{};
        auto si = make_unique<ScalarInit>();
        auto z = make_unique<IntLitExpr>(); z->val = 0;
        si->expr = move(z); def->initVal = move(si);
        decl->defs.push_back(move(def));
        BlockStmt::Item ki; ki.kind = BlockStmt::Item::Kind::Decl; ki.decl = move(decl);
        remBody->items.push_back(move(ki));
    }
    // while (k < kBound) { <原始循环体，其中 j 替换为 j2> k++; }。
    auto remKLoop = make_unique<WhileStmt>();
    remKLoop->cond = cloneExpr(kLoop->cond.get());
    auto remKBody = make_unique<BlockStmt>();
    for (size_t si = 0; si + 1 < kBody->items.size(); ++si) {
        if (kBody->items[si].kind != BlockStmt::Item::Kind::Stmt) continue;
        auto cloned = cloneStmt(kBody->items[si].stmt.get());
        if (!cloned) continue;
        replaceVarInStmt(cloned.get(), colVar, j2Var);
        BlockStmt::Item ci; ci.kind = BlockStmt::Item::Kind::Stmt; ci.stmt = move(cloned);
        remKBody->items.push_back(move(ci));
    }
    {
        BlockStmt::Item ki; ki.kind = BlockStmt::Item::Kind::Stmt;
        ki.stmt = makeIncrement(kVar, 1);
        remKBody->items.push_back(move(ki));
    }
    remKLoop->body = move(remKBody);
    {
        BlockStmt::Item ki; ki.kind = BlockStmt::Item::Kind::Stmt;
        ki.stmt = move(remKLoop);
        remBody->items.push_back(move(ki));
    }
    // C[i][j2] = temp。
    {
        BlockStmt::Item si; si.kind = BlockStmt::Item::Kind::Stmt;
        si.stmt = makeArrayStore(dstArr, rowVar, makeVarPlusInt(j2Var, 0), accVar);
        remBody->items.push_back(move(si));
    }
    // j2++。
    {
        BlockStmt::Item ji; ji.kind = BlockStmt::Item::Kind::Stmt;
        ji.stmt = makeIncrement(j2Var, 1);
        remBody->items.push_back(move(ji));
    }
    remLoop->body = move(remBody);
    {
        BlockStmt::Item li; li.kind = BlockStmt::Item::Kind::Stmt;
        li.stmt = move(remLoop);
        newOuterBody->items.push_back(move(li));
    }

    // 同步原始 j 变量：j = j2，使后续代码看到更新后的值。
    {
        auto sync = make_unique<AssignStmt>();
        auto lv = make_unique<LValExpr>(); lv->name = colVar;
        sync->lval = move(lv);
        auto rv = make_unique<LValExpr>(); rv->name = j2Var;
        sync->rhs = move(rv);
        BlockStmt::Item si; si.kind = BlockStmt::Item::Kind::Stmt; si.stmt = move(sync);
        newOuterBody->items.push_back(move(si));
    }
    // 保留原始 i++。
    newOuterBody->items.push_back(move(outerBody->items[2]));

    // 替换外层循环体。
    outerBody->items = move(newOuterBody->items);
    return true;
}

bool processFunc(FuncDef* func) {
    if (!func || !func->body) return false;
    auto* body = dynamic_cast<BlockStmt*>(func->body.get());
    if (!body) return false;
    for (auto& item : body->items) {
        if (item.kind != BlockStmt::Item::Kind::Stmt) continue;
        auto* whileS = dynamic_cast<WhileStmt*>(item.stmt.get());
        if (!whileS) continue;
        auto* bin = dynamic_cast<BinaryExpr*>(whileS->cond.get());
        if (!bin || bin->op != "<") continue;
        auto* lv = dynamic_cast<LValExpr*>(bin->lhs.get());
        if (!lv || !lv->indices.empty()) continue;
        string iVar = lv->name;
        auto* loopBody = dynamic_cast<BlockStmt*>(whileS->body.get());
        if (loopBody && tryInterchange(loopBody, iVar))
            return true;
    }
    return false;
}

} // namespace

bool applyASTMatmulInterchange(CompUnit& cu) {
    bool changed = false;
    for (auto& item : cu.items) {
        if (item.kind == CompUnit::Item::Kind::FuncDef && item.func) {
            if (processFunc(item.func.get()))
                changed = true;
        }
    }
    return changed;
}

} // namespace hir
