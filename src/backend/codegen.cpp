// 文件职责：实现未优化路径的直接 RISC-V 汇编生成，作为功能性后备后端。本文件负责实现具体逻辑。
#include "codegen.h"
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace {

enum class AbiArgLocKind { FloatReg, IntReg, Stack };

struct AbiArgLoc {
    AbiArgLocKind kind = AbiArgLocKind::Stack;
    int index = -1;
    bool isFloat = false;
    bool isArray = false;
};

struct AbiArgLayout {
    std::vector<AbiArgLoc> locs;
    int intRegsUsed = 0;
    int floatRegsUsed = 0;
    int stackSlotsUsed = 0;
};

static int allocAlignedSlot(int& offset, int bytes, int align) {
    offset -= bytes;
    int used = -offset;
    int rem = used % align;
    if (rem != 0)
        offset -= align - rem;
    return offset;
}

static int slotSizeForType(const SysType& type, bool isParam = false) {
    if (isParam && type.isArray())
        return 8;
    return type.isArray() ? type.totalElems() * 4 : 4;
}

static int slotAlignForType(const SysType& type, bool isParam = false) {
    return (isParam && type.isArray()) ? 8 : 4;
}

static AbiArgLayout assignAbiArgLayout(
    const std::vector<std::pair<BaseType, bool>>& argTypes) {
    AbiArgLayout layout;
    layout.locs.resize(argTypes.size());

    int intReg = 0;
    int floatReg = 0;
    int stackSlot = 0;
    for (size_t i = 0; i < argTypes.size(); ++i) {
        AbiArgLoc loc;
        loc.isFloat = (argTypes[i].first == BaseType::Float && !argTypes[i].second);
        loc.isArray = argTypes[i].second;
        if (loc.isFloat) {
            if (floatReg < 8) {
                loc.kind = AbiArgLocKind::FloatReg;
                loc.index = floatReg++;
            } else if (intReg < 8) {
                loc.kind = AbiArgLocKind::IntReg;
                loc.index = intReg++;
            } else {
                loc.kind = AbiArgLocKind::Stack;
                loc.index = stackSlot++;
            }
        } else {
            if (intReg < 8) {
                loc.kind = AbiArgLocKind::IntReg;
                loc.index = intReg++;
            } else {
                loc.kind = AbiArgLocKind::Stack;
                loc.index = stackSlot++;
            }
        }
        layout.locs[i] = loc;
    }

    layout.intRegsUsed = intReg;
    layout.floatRegsUsed = floatReg;
    layout.stackSlotsUsed = stackSlot;
    return layout;
}

} // namespace

// ============================================================
// 辅助：float → IEEE754 位模式
// ============================================================
static uint32_t floatBits(float f) {
    uint32_t bits;
    memcpy(&bits, &f, 4);
    return bits;
}

// ============================================================
// emit 辅助
// ============================================================
CodeGen::CodeGen(std::ostream& out, bool optimize)
    : out_(out), optimize_(optimize) {}

std::string CodeGen::newLabel(const std::string& prefix) {
    return prefix + std::to_string(labelCnt_++);
}
void CodeGen::emit(const std::string& s) {
    if (funcBufMode_) funcBuf_.push_back(s);
    else out_ << "  " << s << "\n";
}
void CodeGen::emitLabel(const std::string& lbl) {
    if (funcBufMode_) funcBuf_.push_back(lbl + ":");
    else out_ << lbl << ":\n";
}

void CodeGen::emitLoadGlobalAddr(const std::string& reg, const std::string& sym) {
    emit("lla  " + reg + ", " + sym);
}

void CodeGen::normalizeCond(BaseType t) {
    if (t == BaseType::Float) {
        emit("fmv.w.x ft0, zero");
        emit("feq.s a0, fa0, ft0");
        emit("xori a0, a0, 1");
    } else if (t == BaseType::Int) {
        emit("snez a0, a0");
    } else {
        throw std::runtime_error("void expression cannot be used as condition");
    }
}

void CodeGen::beginFuncBuf() {
    funcBufMode_ = true;
    funcBuf_.clear();
}

// ============================================================
// 窥孔优化器
// 规则：
//   模式 1（4 条指令合为 1 条）：addi sp,-8 + sd/fsw REG, 0/4(sp)
//                  + ld/lw/flw REG2, 0/4(sp) + addi sp,+8
//                  → mv REG2, REG  (消除冗余压栈/弹栈)
// ============================================================
static bool isLabel(const std::string& s) {
    return !s.empty() && s.back() == ':' && s.find(' ') == std::string::npos;
}

// 从 "sd   a0, 0(sp)" 或 "fsw  fa0, 4(sp)" 解析寄存器名
static std::string parseStoreReg(const std::string& s,
                                  const std::string& instr,
                                  const std::string& addrPat) {
    // s 需要以 instr 开头，前导空格已由调用者去除。
    if (s.find(instr) != 0) return "";
    auto addrPos = s.find(addrPat);
    if (addrPos == std::string::npos) return "";
    // 寄存器位于指令前缀和逗号之间。
    auto p = instr.size();
    while (p < s.size() && s[p] == ' ') ++p;
    auto q = s.find(',', p);
    if (q == std::string::npos || q >= addrPos) return "";
    auto reg = s.substr(p, q - p);
    // 去除首尾空格。
    while (!reg.empty() && reg.back() == ' ') reg.pop_back();
    return reg;
}

// 从 "lw a1, 0(sp)" 或 "flw fa1, 4(sp)" 解析寄存器名
static std::string parseLoadReg(const std::string& s,
                                 const std::string& instr,
                                 const std::string& addrPat) {
    return parseStoreReg(s, instr, addrPat); // 格式相同
}

// ---- 解析辅助 ----
// 从 "instr   REG, OFF(BASE)" 中提取 REG 和 "OFF(BASE)" 部分
static bool parseMemInstr(const std::string& s, const std::string& instrPfx,
                           std::string& reg, std::string& offBase) {
    if (s.find(instrPfx) != 0) return false;
    size_t p = instrPfx.size();
    while (p < s.size() && s[p] == ' ') ++p;
    size_t q = s.find(',', p);
    if (q == std::string::npos) return false;
    reg = s.substr(p, q - p);
    while (!reg.empty() && reg.back() == ' ') reg.pop_back();
    p = q + 2; // skip ", "
    offBase = s.substr(p);
    while (!offBase.empty() && offBase.back() == '\n') offBase.pop_back();
    while (!offBase.empty() && offBase.front() == ' ') offBase = offBase.substr(1);
    return !reg.empty() && !offBase.empty();
}

// 从 "li REG, VAL" 中提取 REG 和 VAL
static bool parseLiInstr(const std::string& s, std::string& reg, int& val) {
    if (s.size() < 4) return false;
    if (s.substr(0, 3) != "li ") return false;
    size_t p = 3;
    while (p < s.size() && s[p] == ' ') ++p;
    size_t q = s.find(',', p);
    if (q == std::string::npos) return false;
    reg = s.substr(p, q - p);
    while (!reg.empty() && reg.back() == ' ') reg.pop_back();
    p = q + 1;
    while (p < s.size() && s[p] == ' ') ++p;
    try { val = std::stoi(s.substr(p)); return true; }
    catch (...) { return false; }
}

// 检查 buf[lo..hi] 区间内是否没有标签
static bool noLabels(const std::vector<std::string>& buf, int lo, int hi) {
    for (int k = lo; k <= hi; ++k)
        if (k < (int)buf.size() && isLabel(buf[k])) return false;
    return true;
}

void CodeGen::flushFuncBuf() {
    // 多轮窥孔优化（2 轮处理级联情形）
    for (int pass = 0; pass < 2; ++pass) {
        std::vector<std::string> out;
        out.reserve(funcBuf_.size());
        const auto& buf = funcBuf_;
        int n = (int)buf.size();
        int i = 0;

        while (i < n) {
            bool matched = false;

            // ================================================================
            // 形状 3（12→3）：var = var + CONST_SMALL，例如 i = i + 1。
            //   lw a0, OFF(s0)      ← 加载变量
            //   addi sp,sp,-8
            //   sd a0, 0(sp)
            //   li a0, CONST        ← 小常量
            //   lw a1, 0(sp)
            //   addi sp,sp,8
            //   addw a0, a1, a0     ← var + CONST
            //   addi sp,sp,-8       ← 赋值入栈
            //   sd a0, 0(sp)
            //   ld t1, 0(sp)        ← 赋值出栈
            //   addi sp,sp,8
            //   sw t1, OFF(s0)      ← 写回，偏移与加载相同
            // → lw a0, OFF(s0); addi a0, a0, CONST; sw a0, OFF(s0)
            // ================================================================
            if (!matched && i + 11 < n && noLabels(buf, i + 1, i + 11)) {
                std::string reg0, ob0; // line i
                std::string reg3, ob3; // line i+3 (li)
                int constVal = 0;
                std::string regLoad11, ob11; // line i+11 (sw t1,OFF)
                if (parseMemInstr(buf[i], "lw", reg0, ob0) && reg0 == "a0" &&
                    ob0.find("(s0)") != std::string::npos &&
                    buf[i+1] == "addi sp, sp, -8" &&
                    buf[i+2] == "sd   a0, 0(sp)" &&
                    parseLiInstr(buf[i+3], reg3, constVal) && reg3 == "a0" &&
                    constVal >= -2047 && constVal <= 2047 &&
                    buf[i+4] == "lw   a1, 0(sp)" &&
                    buf[i+5] == "addi sp, sp, 8" &&
                    buf[i+6] == "addw  a0, a1, a0" &&
                    buf[i+7] == "addi sp, sp, -8" &&
                    buf[i+8] == "sd   a0, 0(sp)" &&
                    buf[i+9] == "ld   t1, 0(sp)" &&
                    buf[i+10] == "addi sp, sp, 8" &&
                    parseMemInstr(buf[i+11], "sw", regLoad11, ob11) &&
                    regLoad11 == "t1" && ob11 == ob0) {
                    // 生成：lw a0, OFF(s0); addi a0, a0, CONST; sw a0, OFF(s0)。
                    out.push_back(buf[i]); // lw a0, OFF(s0)
                    out.push_back("addi a0, a0, " + std::to_string(constVal));
                    out.push_back("sw   a0, " + ob0);
                    i += 12; matched = true;
                }
            }

            // ================================================================
            // 形状 2（7→3）：lw a0,X(s0) + push + lw a0,Y + pop a1 + 二元运算。
            //   lw a0, X(s0)     ← 加载变量 A（相对 s0）
            //   addi sp,sp,-8
            //   sd a0, 0(sp)
            //   lw/ld a0, Y(?)   ← 加载变量 B（基址不能是 sp）
            //   lw a1, 0(sp)
            //   addi sp,sp,8
            //   OP a0, a1, a0    ← A OP B
            // → lw a1, X(s0); lw a0, Y(?); OP a0, a1, a0
            // ================================================================
            if (!matched && i + 6 < n && noLabels(buf, i + 1, i + 6)) {
                std::string regI, obI; // line i
                std::string regI3, obI3; // line i+3
                if (parseMemInstr(buf[i], "lw", regI, obI) && regI == "a0" &&
                    obI.find("(s0)") != std::string::npos &&
                    buf[i+1] == "addi sp, sp, -8" &&
                    buf[i+2] == "sd   a0, 0(sp)" &&
                    (parseMemInstr(buf[i+3], "lw", regI3, obI3) ||
                     parseMemInstr(buf[i+3], "ld", regI3, obI3)) &&
                    regI3 == "a0" && obI3.find("(sp)") == std::string::npos &&
                    buf[i+4] == "lw   a1, 0(sp)" &&
                    buf[i+5] == "addi sp, sp, 8") {
                    // 替换：lw a1, X(s0) + [第 i+3 行不变] + [第 i+6 行不变]。
                    std::string newLoad = buf[i];
                    // 将第一条加载中的 a0 改为 a1。
                    auto pos = newLoad.find("a0");
                    if (pos != std::string::npos) newLoad.replace(pos, 2, "a1");
                    out.push_back(newLoad);    // lw a1, X(s0)
                    out.push_back(buf[i+3]);   // lw a0, Y(?)
                    out.push_back(buf[i+6]);   // OP a0, a1, a0
                    i += 7; matched = true;
                }
            }

            // ================================================================
            // 形状 2b（6→3）：push a0 + lw/ld a0,Y + pop a1 + 二元运算。
            //   addi sp,sp,-8
            //   sd a0, 0(sp)     ← 保存当前 a0
            //   lw/ld a0, Y(?)   ← 加载变量 B（基址不能是 sp）
            //   lw a1, 0(sp)
            //   addi sp,sp,8
            //   OP a0, a1, a0    ← current_a0 OP B
            // → mv a1, a0; lw a0, Y(?); OP a0, a1, a0
            // ================================================================
            if (!matched && i + 5 < n && noLabels(buf, i + 1, i + 5)) {
                std::string regI2, obI2;
                if (buf[i] == "addi sp, sp, -8" &&
                    buf[i+1] == "sd   a0, 0(sp)" &&
                    (parseMemInstr(buf[i+2], "lw", regI2, obI2) ||
                     parseMemInstr(buf[i+2], "ld", regI2, obI2)) &&
                    regI2 == "a0" && obI2.find("(sp)") == std::string::npos &&
                    buf[i+3] == "lw   a1, 0(sp)" &&
                    buf[i+4] == "addi sp, sp, 8") {
                    out.push_back("mv   a1, a0");
                    out.push_back(buf[i+2]);   // lw a0, Y(?)
                    out.push_back(buf[i+5]);   // OP a0, a1, a0
                    i += 6; matched = true;
                }
            }

            // ================================================================
            // 形状 1（4→1）：addi sp,-8 + sd/fsw REG + ld/lw/flw REG2 + addi sp,+8。
            //   （连续入栈后立即出栈）
            // → mv REG2, REG（浮点使用 fmv.s；寄存器相同时直接删除）
            // ================================================================
            if (!matched && i + 3 < n && noLabels(buf, i + 1, i + 3)) {
                const auto& a = buf[i];
                const auto& b = buf[i+1];
                const auto& c = buf[i+2];
                const auto& d = buf[i+3];
                if (a == "addi sp, sp, -8" && d == "addi sp, sp, 8") {
                    std::string regB, obB, regC, obC;
                    bool intPat = (parseMemInstr(b, "sd", regB, obB) && obB == "0(sp)" &&
                                   (parseMemInstr(c, "ld", regC, obC) ||
                                    parseMemInstr(c, "lw", regC, obC)) && obC == "0(sp)");
                    bool fltPat = (!intPat &&
                                   parseMemInstr(b, "fsw", regB, obB) && obB == "4(sp)" &&
                                   parseMemInstr(c, "flw", regC, obC) && obC == "4(sp)");
                    if (intPat || fltPat) {
                        if (regB != regC) {
                            std::string mvInstr = fltPat ? "fmv.s " : "mv   ";
                            out.push_back(mvInstr + regC + ", " + regB);
                        }
                        // 如果入栈和出栈使用同一寄存器，则整个过程是恒等操作，跳过四条指令。
                        i += 4; matched = true;
                    }
                }
            }

            // ================================================================
            // 形状 4（2→1）：mv t1, REG + sw t1, X(base) → sw REG, X(base)。
            //   （消除形状 1 产生的多余 mv+sw 对）
            // ================================================================
            if (!matched && i + 1 < n && !isLabel(buf[i+1])) {
                const auto& mv_line = buf[i];
                const auto& sw_line = buf[i+1];
                std::string mvDst, mvSrc;
                // mv dst, src 解析为 "mv   DST, SRC"。
                if (mv_line.size() > 4 && mv_line.substr(0, 3) == "mv ") {
                    size_t p = 3;
                    while (p < mv_line.size() && mv_line[p] == ' ') ++p;
                    size_t q = mv_line.find(',', p);
                    if (q != std::string::npos) {
                        mvDst = mv_line.substr(p, q - p);
                        while (!mvDst.empty() && mvDst.back() == ' ') mvDst.pop_back();
                        p = q + 1;
                        while (p < mv_line.size() && mv_line[p] == ' ') ++p;
                        mvSrc = mv_line.substr(p);
                        while (!mvSrc.empty() && mvSrc.back() == ' ') mvSrc.pop_back();
                    }
                }
                if (!mvDst.empty() && !mvSrc.empty()) {
                    std::string swReg, swAddr;
                    if (parseMemInstr(sw_line, "sw", swReg, swAddr) && swReg == mvDst) {
                        out.push_back("sw   " + mvSrc + ", " + swAddr);
                        i += 2; matched = true;
                    }
                    if (!matched) {
                        // 同时处理 fsw ft1 → fsw fa0 的形状。
                        if (parseMemInstr(sw_line, "fsw", swReg, swAddr) && swReg == mvDst) {
                            out.push_back("fsw  " + mvSrc + ", " + swAddr);
                            i += 2; matched = true;
                        }
                    }
                }
            }

            // ================================================================
            // 形状 6（3→2）：OP a0,a1,a0 + mv a1,a0 + lw a0,X → OP a1,a1,a0 + lw a0,X。
            //   （当 OP 可交换时，避免 mv，直接写入 a1）
            // ================================================================
            if (!matched && i + 2 < n && noLabels(buf, i + 1, i + 2)) {
                const auto& op_l  = buf[i];
                const auto& mv_l  = buf[i+1];
                const auto& lw_l  = buf[i+2];
                // 检查操作：可交换二元运算 "OP a0, a1, a0"。
                bool isComm = (op_l == "addw  a0, a1, a0" ||
                                op_l == "mulw  a0, a1, a0");
                // 检查 mv："mv   a1, a0"。
                bool isMvA1A0 = (mv_l == "mv   a1, a0");
                // 检查 lw：加载到 a0，基址不能是 sp。
                std::string lwReg, lwAddr;
                bool isLw = ((parseMemInstr(lw_l, "lw", lwReg, lwAddr) ||
                               parseMemInstr(lw_l, "ld", lwReg, lwAddr)) &&
                              lwReg == "a0" && lwAddr.find("(sp)") == std::string::npos);
                if (isComm && isMvA1A0 && isLw) {
                    // 将 "OP a0, a1, a0" 替换为 "OP a1, a1, a0"。
                    std::string newOp = op_l;
                    // 将第一个 a0（目标寄存器）改为 a1。
                    size_t pos = newOp.find("a0");
                    if (pos != std::string::npos) newOp.replace(pos, 2, "a1");
                    out.push_back(newOp);  // OP a1, a1, a0
                    out.push_back(lw_l);   // lw a0, X
                    i += 3; matched = true;
                }
            }

            // ================================================================
            // Pattern 5 (2→1): 合并 slt+beqz/bnez 为单条分支
            //   slt a0, a1, a0 + beqz a0, L  → bge a1, a0, L  (a1 >= a0 → jump)
            //   slt a0, a1, a0 + bnez a0, L  → blt a1, a0, L  (a1 <  a0 → jump)
            //   slt a0, a0, a1 + beqz a0, L  → bge a0, a1, L
            //   slt a0, a0, a1 + bnez a0, L  → blt a0, a1, L
            // ================================================================
            if (!matched && i + 1 < n && !isLabel(buf[i]) && !isLabel(buf[i+1])) {
                const auto& slt_l = buf[i];
                const auto& br_l  = buf[i+1];
                // 匹配 slt a0, RS1, RS2。
                std::string rs1, rs2;
                bool sltOk = false;
                if (slt_l.size() > 8) {
                    // "slt  a0, RS1, RS2"
                    if (slt_l.substr(0, 4) == "slt ") {
                        size_t p = 4;
                        while (p < slt_l.size() && slt_l[p]==' ') ++p;
                        size_t q1 = slt_l.find(',', p);
                        if (q1 != std::string::npos) {
                            std::string dst = slt_l.substr(p, q1-p);
                            while (!dst.empty() && dst.back()==' ') dst.pop_back();
                            if (dst == "a0") {
                                p = q1+1;
                                while (p<slt_l.size() && slt_l[p]==' ') ++p;
                                size_t q2 = slt_l.find(',', p);
                                if (q2 != std::string::npos) {
                                    rs1 = slt_l.substr(p, q2-p);
                                    while (!rs1.empty() && rs1.back()==' ') rs1.pop_back();
                                    p = q2+1;
                                    while (p<slt_l.size() && slt_l[p]==' ') ++p;
                                    rs2 = slt_l.substr(p);
                                    while (!rs2.empty() && rs2.back()==' ') rs2.pop_back();
                                    sltOk = !rs1.empty() && !rs2.empty();
                                }
                            }
                        }
                    }
                }
                if (sltOk) {
                    // 匹配 beqz/bnez a0, LABEL。
                    std::string label;
                    bool isBEQZ = (br_l.substr(0,5) == "beqz ");
                    bool isBNEZ = (br_l.substr(0,5) == "bnez ");
                    if ((isBEQZ || isBNEZ) && br_l.find("a0,") != std::string::npos) {
                        size_t lp = br_l.find("a0,") + 3;
                        while (lp < br_l.size() && br_l[lp]==' ') ++lp;
                        label = br_l.substr(lp);
                        while (!label.empty() && label.back()==' ') label.pop_back();
                        if (!label.empty()) {
                            // slt a0, rs1, rs2 = a0 = (rs1 < rs2)
                            // beqz a0 = jump if !(rs1<rs2) = if rs1>=rs2 → bge rs1, rs2
                            // bnez a0 = jump if  (rs1<rs2)               → blt rs1, rs2
                            std::string brInstr = isBEQZ ? "bge  " : "blt  ";
                            out.push_back(brInstr + rs1 + ", " + rs2 + ", " + label);
                            i += 2; matched = true;
                        }
                    }
                }
            }

            if (!matched) { out.push_back(buf[i]); ++i; }
        }
        funcBuf_ = out;
    }

    // 输出
    for (const auto& line : funcBuf_) {
        if (isLabel(line)) out_ << line << "\n";
        else               out_ << "  " << line << "\n";
    }
    funcBuf_.clear();
    funcBufMode_ = false;
}

// ============================================================
// 符号表
// ============================================================
void CodeGen::pushScope() { scopes_.push_back({}); }
void CodeGen::popScope()  { scopes_.pop_back(); }

Symbol* CodeGen::lookupSym(const std::string& name) {
    for (int i = (int)scopes_.size()-1; i >= 0; --i) {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end()) return &it->second;
    }
    auto it = globals_.find(name);
    if (it != globals_.end()) return &it->second;
    return nullptr;
}
void CodeGen::addSym(Symbol sym) {
    if (scopes_.empty()) globals_[sym.name] = std::move(sym);
    else scopes_.back()[sym.name] = std::move(sym);
}

// ============================================================
// 栈空间分配（相对于 s0 的负偏移，按对象所需对齐）
// ============================================================
int CodeGen::allocLocal(int bytes, int align) {
    return allocAlignedSlot(nextOffset_, bytes, align);
}

// ============================================================
// 常量求值（编译期）
// ============================================================
int32_t CodeGen::evalConstInt(Expr* e) {
    if (auto* x = dynamic_cast<IntLitExpr*>(e))   return x->val;
    if (auto* x = dynamic_cast<FloatLitExpr*>(e)) return (int32_t)x->val;
    if (auto* x = dynamic_cast<UnaryExpr*>(e)) {
        int32_t v = evalConstInt(x->operand.get());
        if (x->op == '-') return -v;
        if (x->op == '!') return !v;
        if (x->op == '~') return ~v;
        return v;
    }
    if (auto* x = dynamic_cast<BinaryExpr*>(e)) {
        int32_t l = evalConstInt(x->lhs.get());
        int32_t r = evalConstInt(x->rhs.get());
        if (x->op == "+")  return l + r;
        if (x->op == "-")  return l - r;
        if (x->op == "*")  return l * r;
        if (x->op == "/")  return r ? l / r : 0;
        if (x->op == "%")  return r ? l % r : 0;
        if (x->op == "&")  return l & r;
        if (x->op == "|")  return l | r;
        if (x->op == "^")  return l ^ r;
        if (x->op == "<")  return l < r;
        if (x->op == "<=") return l <= r;
        if (x->op == ">")  return l > r;
        if (x->op == ">=") return l >= r;
        if (x->op == "==") return l == r;
        if (x->op == "!=") return l != r;
        if (x->op == "&&") return l && r;
        if (x->op == "||") return l || r;
    }
    if (auto* x = dynamic_cast<LValExpr*>(e)) {
        Symbol* sym = lookupSym(x->name);
        if (sym && sym->isConst) {
            if (x->indices.empty() && sym->constInt.has_value())
                return *sym->constInt;
            if (!x->indices.empty() && !sym->constIntArr.empty()) {
                int idx = evalConstInt(x->indices[0].get());
                if (idx >= 0 && idx < (int)sym->constIntArr.size())
                    return sym->constIntArr[idx];
            }
        }
    }
    throw std::runtime_error("not a compile-time integer constant");
}

float CodeGen::evalConstFloat(Expr* e) {
    if (auto* x = dynamic_cast<FloatLitExpr*>(e)) return x->val;
    if (auto* x = dynamic_cast<IntLitExpr*>(e))   return (float)x->val;
    if (auto* x = dynamic_cast<UnaryExpr*>(e)) {
        float v = evalConstFloat(x->operand.get());
        return (x->op == '-') ? -v : v;
    }
    if (auto* x = dynamic_cast<BinaryExpr*>(e)) {
        float l = evalConstFloat(x->lhs.get());
        float r = evalConstFloat(x->rhs.get());
        if (x->op == "+") return l + r;
        if (x->op == "-") return l - r;
        if (x->op == "*") return l * r;
        if (x->op == "/") return l / r;
    }
    if (auto* x = dynamic_cast<LValExpr*>(e)) {
        Symbol* sym = lookupSym(x->name);
        if (sym && sym->isConst && sym->constFloat.has_value() && x->indices.empty())
            return *sym->constFloat;
    }
    return (float)evalConstInt(e);
}

// ============================================================
// 加载立即数
// 直接用 RISC-V 汇编伪指令 `li`，汇编器会自动展开为正确的指令序列
// 这样避免了手动计算 lui+addi 时对负数的边界情况处理错误
// ============================================================
void CodeGen::loadImm(const std::string& reg, int64_t val) {
    emit("li " + reg + ", " + std::to_string(val));
}

// ============================================================
// 大偏移量辅助（RISC-V 立即数字段只有 12-bit，范围 -2048~2047）
// 所有对栈帧变量（s0 相对）的访问都通过这组函数，
// 确保偏移量超出 12-bit 范围时自动使用 li+add 中转。
// 使用 t6 作为临时计算寄存器（caller-saved，不影响参数/结果）。
// ============================================================
void CodeGen::emitAddrAdd(const std::string& dst,
                           const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("addi " + dst + ", " + base + ", " + std::to_string(offset));
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  " + dst + ", " + base + ", t6");
    }
}

void CodeGen::emitLw(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("lw   " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("lw   " + reg + ", 0(t6)");
    }
}

void CodeGen::emitFlw(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("flw  " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("flw  " + reg + ", 0(t6)");
    }
}

void CodeGen::emitLd(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("ld   " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("ld   " + reg + ", 0(t6)");
    }
}

void CodeGen::emitSw(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("sw   " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("sw   " + reg + ", 0(t6)");
    }
}

void CodeGen::emitFsw(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("fsw  " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("fsw  " + reg + ", 0(t6)");
    }
}

void CodeGen::emitSd(const std::string& reg, const std::string& base, int offset) {
    if (isSmallImm(offset)) {
        emit("sd   " + reg + ", " + std::to_string(offset) + "(" + base + ")");
    } else {
        emit("li   t6, " + std::to_string(offset));
        emit("add  t6, " + base + ", t6");
        emit("sd   " + reg + ", 0(t6)");
    }
}

// float 常量 → fa0（通过整数寄存器中转）
void CodeGen::loadFloatConst(float val) {
    uint32_t bits = floatBits(val);
    loadImm("a0", (int32_t)bits);
    emit("fmv.w.x fa0, a0");
}

// ============================================================
// 类型转换
// ============================================================
void CodeGen::intToFloat()  { emit("fcvt.s.w fa0, a0"); }
void CodeGen::floatToInt()  { emit("fcvt.w.s a0, fa0, rtz"); } // 向零截断

// ============================================================
// 整型二元运算：a1 op a0 → a0。算术使用 *w 保证 32 位语义；输入值均已
// 按 i32 符号扩展，RV64 按位指令会保持结果的规范符号扩展形式。
// ============================================================
void CodeGen::emitIntBinop(const std::string& op) {
    if (op == "+")      emit("addw  a0, a1, a0");
    else if (op == "-") emit("subw  a0, a1, a0");
    else if (op == "*") emit("mulw  a0, a1, a0");
    else if (op == "/") emit("divw  a0, a1, a0");
    else if (op == "%") emit("remw  a0, a1, a0");
    else if (op == "&") emit("and   a0, a1, a0");
    else if (op == "|") emit("or    a0, a1, a0");
    else if (op == "^") emit("xor   a0, a1, a0");
    else throw std::runtime_error("unknown int binop: " + op);
}

// 浮点二元运算：fa1 op fa0 → fa0
void CodeGen::emitFloatBinop(const std::string& op) {
    if (op == "+")      emit("fadd.s fa0, fa1, fa0");
    else if (op == "-") emit("fsub.s fa0, fa1, fa0");
    else if (op == "*") emit("fmul.s fa0, fa1, fa0");
    else if (op == "/") emit("fdiv.s fa0, fa1, fa0");
    else throw std::runtime_error("unknown float binop: " + op);
}

// 整型比较：a1 op a0 → a0（0 or 1）
void CodeGen::emitIntCmp(const std::string& op) {
    if (op == "==")      { emit("sub  a0, a1, a0"); emit("seqz a0, a0"); }
    else if (op == "!=") { emit("sub  a0, a1, a0"); emit("snez a0, a0"); }
    else if (op == "<")  { emit("slt  a0, a1, a0"); }
    else if (op == "<=") { emit("slt  a0, a0, a1"); emit("xori a0, a0, 1"); }
    else if (op == ">")  { emit("slt  a0, a0, a1"); }
    else if (op == ">=") { emit("slt  a0, a1, a0"); emit("xori a0, a0, 1"); }
}

// 浮点比较：fa1 op fa0 → a0（0 or 1）
void CodeGen::emitFloatCmp(const std::string& op) {
    if (op == "==")      emit("feq.s a0, fa1, fa0");
    else if (op == "!=") { emit("feq.s a0, fa1, fa0"); emit("xori a0, a0, 1"); }
    else if (op == "<")  emit("flt.s a0, fa1, fa0");
    else if (op == "<=") emit("fle.s a0, fa1, fa0");
    else if (op == ">")  emit("flt.s a0, fa0, fa1");
    else if (op == ">=") emit("fle.s a0, fa0, fa1");
}

// ============================================================
// generate: 总入口
// ============================================================
void CodeGen::generate(CompUnit* cu) {
    // 不发射硬编码的 .file "program.sy"，避免把源文件名写入汇编元数据。
    out_ << "\n";

    // 预扫描：注册所有函数的返回类型和参数类型（用于调用时确定返回值与隐式转换）
    // 内置运行时函数（参数类型用于隐式转换）
    auto addBuiltin = [&](const std::string& name, BaseType ret,
                          std::vector<std::pair<BaseType, bool>> params = {}) {
        funcRetTypes_[name] = ret;
        funcParamTypes_[name] = std::move(params);
    };
    addBuiltin("getint",    BaseType::Int);
    addBuiltin("getch",     BaseType::Int);
    addBuiltin("getarray",  BaseType::Int,    {{BaseType::Int, true}});
    addBuiltin("putint",    BaseType::Void,   {{BaseType::Int, false}});
    addBuiltin("putch",     BaseType::Void,   {{BaseType::Int, false}});
    addBuiltin("putarray",  BaseType::Void,   {{BaseType::Int, false}, {BaseType::Int, true}});
    addBuiltin("getfloat",  BaseType::Float);
    addBuiltin("getfarray", BaseType::Int,    {{BaseType::Float, true}});
    addBuiltin("putfloat",  BaseType::Void,   {{BaseType::Float, false}});
    addBuiltin("putfarray", BaseType::Void,   {{BaseType::Int, false}, {BaseType::Float, true}});
    addBuiltin("getstr", BaseType::Void,
               {{BaseType::Int, true}, {BaseType::Int, false}});
    addBuiltin("putstr", BaseType::Void, {{BaseType::Int, true}});

    for (auto& item : cu->items)
        if (item.kind == CompUnit::Item::Kind::FuncDef) {
            userFunctions_.insert(item.func->name);
            funcRetTypes_[item.func->name] = item.func->retType.base;
            std::vector<std::pair<BaseType, bool>> ptypes;
            for (auto& p : item.func->params)
                ptypes.push_back({p->type.base, p->type.isArray()});
            funcParamTypes_[item.func->name] = std::move(ptypes);
        }

    // 第一遍：全局变量
    for (auto& item : cu->items)
        if (item.kind == CompUnit::Item::Kind::GlobalDecl)
            genGlobal(item.decl.get());

    // 第二遍：函数
    out_ << "\n  .text\n";
    for (auto& item : cu->items)
        if (item.kind == CompUnit::Item::Kind::FuncDef)
            genFunc(item.func.get());
}

// ============================================================
// 全局变量/常量生成
// ============================================================
void CodeGen::genGlobal(DeclStmt* decl) {
    for (auto& def : decl->defs) {
        Symbol sym;
        sym.name    = def->name;
        sym.type    = def->type;
        sym.type.base = decl->baseType;
        sym.isConst = decl->isConst;
        sym.isGlobal = true;

        // const 标量：编译期内联，不占全局存储
        if (decl->isConst && !def->type.isArray()) {
            auto* sc = dynamic_cast<ScalarInit*>(def->initVal.get());
            if (decl->baseType == BaseType::Int)
                sym.constInt = evalConstInt(sc->expr.get());
            else
                sym.constFloat = evalConstFloat(sc->expr.get());
            globals_[sym.name] = sym;
            continue;
        }

        int totalElems = def->type.isArray() ? def->type.totalElems() : 1;
        int bytes      = totalElems * 4;

        bool emptyAggregateInit = false;
        if (auto* agg = dynamic_cast<AggregateInit*>(def->initVal.get()))
            emptyAggregateInit = agg->items.empty();

        if (!def->initVal || emptyAggregateInit) {
            // 未初始化或 {} 零初始化全局 → .bss（全零）
            out_ << "\n  .bss\n  .globl " << def->name << "\n";
            out_ << "  .align 2\n" << def->name << ":\n";
            out_ << "  .zero " << bytes << "\n";
        } else {
            out_ << "\n  .data\n  .globl " << def->name << "\n";
            out_ << "  .align 2\n" << def->name << ":\n";
            int idx = 0;
            emitGlobalData(sym, def->initVal.get(), idx, totalElems, 0);
            // 剩余元素补零
            while (idx < totalElems) { out_ << "  .word 0\n"; ++idx; }
        }
        globals_[sym.name] = sym;
    }
}

void CodeGen::emitGlobalData(const Symbol& sym, InitVal* init,
                              int& idx, int total, int depth) {
    if (auto* sc = dynamic_cast<ScalarInit*>(init)) {
        if (idx >= total) return;
        if (sym.type.base == BaseType::Float) {
            float fv = 0.0f;
            try { fv = evalConstFloat(sc->expr.get()); } catch(...) {}
            out_ << "  .word " << floatBits(fv) << "\n";
        } else {
            int32_t iv = 0;
            try { iv = evalConstInt(sc->expr.get()); } catch(...) {}
            out_ << "  .word " << iv << "\n";
        }
        ++idx;
    } else if (auto* agg = dynamic_cast<AggregateInit*>(init)) {
        // 计算当前层的子数组大小：dims[depth+1] * ... （与 genArrayInit 逻辑一致）
        const auto& dims = sym.type.dims;
        int subSize = 1;
        for (int d = depth + 1; d < (int)dims.size(); ++d)
            if (dims[d] > 0) subSize *= dims[d];

        for (auto& item : agg->items) {
            if (auto* nested = dynamic_cast<AggregateInit*>(item.get())) {
                // 嵌套大括号：先对齐到子数组边界，跳过的位置补零
                if (subSize > 1 && idx % subSize != 0) {
                    int alignedIdx = (idx / subSize + 1) * subSize;
                    while (idx < alignedIdx && idx < total) {
                        out_ << "  .word 0\n"; ++idx;
                    }
                }
                int baseIdx = idx;
                emitGlobalData(sym, item.get(), idx, total, depth + 1);
                // 推进到本子数组末尾，子数组未初始化的尾部补零
                if (subSize > 1) {
                    int endIdx = baseIdx / subSize * subSize + subSize;
                    while (idx < endIdx && idx < total) {
                        out_ << "  .word 0\n"; ++idx;
                    }
                    if (idx < endIdx) idx = endIdx; // 超出 total 时也推进 idx
                }
            } else {
                emitGlobalData(sym, item.get(), idx, total, depth + 1);
            }
        }
    } else if (auto* str = dynamic_cast<StringInit*>(init)) {
        for (unsigned char c : str->value) {
            if (idx >= total) break;
            out_ << "  .word " << static_cast<int>(c) << "\n";
            ++idx;
        }
        if (idx < total) {
            out_ << "  .word 0\n";
            ++idx;
        }
    }
}

// ============================================================
// 函数生成
// ============================================================
void CodeGen::genFunc(FuncDef* func) {
    curRetType_     = func->retType;
    curFuncName_    = func->name;
    breakLabel_     = "";
    continueLabel_  = "";
    nextOffset_     = -16; // 前 16 字节给 ra+s0

    // ---- 预扫描：计算帧大小 ----
    // 1. 参数（存入栈槽）
    pushScope(); // 函数作用域
    std::vector<std::pair<std::string, Symbol>> paramSyms;
    std::vector<std::pair<BaseType, bool>> paramTypes;
    for (auto& p : func->params) {
        int off = allocLocal(slotSizeForType(p->type, true),
                             slotAlignForType(p->type, true));
        Symbol psym;
        psym.name    = p->name;
        psym.type    = p->type;
        psym.isParam = true;
        psym.fpOffset = off;
        paramSyms.push_back({p->name, psym});
        paramTypes.push_back({p->type.base, p->type.isArray()});
    }
    // 2. 局部变量声明
    int savedNextOff = nextOffset_;
    scanDecls(func->body.get(), nextOffset_);
    int minOffset = nextOffset_;

    // 帧大小 = 16（ra+s0）+ 所有局部变量 + 对齐到 16 字节
    frameSize_ = (int)(-minOffset + 16);
    if (frameSize_ % 16 != 0)
        frameSize_ = (frameSize_ / 16 + 1) * 16;

    // 重置偏移到参数分配完毕的位置（genDecl 会重新从这里分配）
    nextOffset_ = savedNextOff;

    // 注册参数符号
    for (auto& [name, sym] : paramSyms)
        addSym(sym);

    // 生成函数标签
    out_ << "\n  .globl " << func->name << "\n";
    out_ << "  .type  " << func->name << ", @function\n";
    out_ << func->name << ":\n";

    // 启动指令缓冲（用于窥孔优化）
    beginFuncBuf();

    // ---- 函数序言 ----
    // 小帧（<=2047字节）：4条指令，高效路径
    // 大帧（>2047字节）：用 li+t3 中转，兼容任意帧大小
    if (isSmallImm(frameSize_)) {
        emit("addi sp, sp, -" + std::to_string(frameSize_));
        emit("sd   ra, " + std::to_string(frameSize_ - 8) + "(sp)");
        emit("sd   s0, " + std::to_string(frameSize_ - 16) + "(sp)");
        emit("addi s0, sp, " + std::to_string(frameSize_));
    } else {
        emit("li   t3, " + std::to_string(frameSize_));
        emit("sub  sp, sp, t3");
        emit("add  t3, sp, t3");
        emit("sd   ra, -8(t3)");
        emit("sd   s0, -16(t3)");
        emit("mv   s0, t3");
    }

    // RV64GC lp64d ABI：
    //   float 标量优先走 fa0-fa7，用尽后回退到 a0-a7，再之后才上栈；
    //   int 标量和数组退化指针只走 a0-a7，用尽后上栈。
    AbiArgLayout paramLayout = assignAbiArgLayout(paramTypes);
    for (int i = 0; i < (int)func->params.size(); ++i) {
        auto& p = func->params[i];
        Symbol* psym = lookupSym(p->name);
        const AbiArgLoc& loc = paramLayout.locs[i];
        if (loc.kind == AbiArgLocKind::FloatReg) {
            emitFsw("fa" + std::to_string(loc.index), "s0", psym->fpOffset);
            continue;
        }

        if (loc.kind == AbiArgLocKind::IntReg) {
            std::string srcReg = "a" + std::to_string(loc.index);
            if (p->type.isArray())
                emitSd(srcReg, "s0", psym->fpOffset);
            else
                emitSw(srcReg, "s0", psym->fpOffset);
            continue;
        }

        int stackOffset = loc.index * 8;
        if (p->type.isArray()) {
            emitLd("t0", "s0", stackOffset);
            emitSd("t0", "s0", psym->fpOffset);
        } else if (loc.isFloat) {
            emitLw("t0", "s0", stackOffset);
            emitSw("t0", "s0", psym->fpOffset);
        } else {
            emitLw("t0", "s0", stackOffset);
            emitSw("t0", "s0", psym->fpOffset);
        }
    }

    // 返回标签（所有 return 语句都跳转到此）
    curRetLabel_ = newLabel(".Lret_" + func->name + "_");

    // ---- 生成函数体 ----
    genBlock(func->body.get());

    // 函数末尾：隐式 return（若 int 函数无显式 return，默认返回 0）
    // 这条指令仅在代码落穿到 epilogue 时执行（显式 return 通过 j 直接跳到 label 后）
    if (func->retType.base == BaseType::Float)
        emit("fmv.w.x fa0, zero");
    else if (func->retType.base != BaseType::Void)
        emit("li   a0, 0");           // int 函数：隐式返回 0（如 main 无 return）

    // ---- 函数尾声 ----
    emitLabel(curRetLabel_);
    if (isSmallImm(frameSize_)) {
        emit("ld   ra, " + std::to_string(frameSize_ - 8) + "(sp)");
        emit("ld   s0, " + std::to_string(frameSize_ - 16) + "(sp)");
        emit("addi sp, sp, " + std::to_string(frameSize_));
    } else {
        emit("ld   ra, -8(s0)");
        emit("ld   t3, -16(s0)");
        emit("mv   sp, s0");
        emit("mv   s0, t3");
    }
    emit("ret");

    // 功能测试路径优先保守正确，只有显式 -O1 时才启用窥孔优化。
    if (optimize_) {
        flushFuncBuf();
    } else {
        for (const auto& line : funcBuf_) {
            if (isLabel(line)) out_ << line << "\n";
            else               out_ << "  " << line << "\n";
        }
        funcBuf_.clear();
        funcBufMode_ = false;
    }
    out_ << "  .size " << func->name << ", .-" << func->name << "\n";

    popScope();
}

// 预扫描局部变量（只计算空间，不注册符号）
void CodeGen::scanDecls(BlockStmt* block, int& offset) {
    for (auto& item : block->items) {
        if (item.kind == BlockStmt::Item::Kind::Decl) {
            auto& decl = item.decl;
            for (auto& def : decl->defs) {
                if (decl->isStatic) continue;
                // const 标量不占栈
                if (decl->isConst && !def->type.isArray()) continue;
                allocAlignedSlot(offset,
                                 slotSizeForType(def->type),
                                 slotAlignForType(def->type));
            }
        } else {
            scanStmt(item.stmt.get(), offset);
        }
    }
}

void CodeGen::scanStmt(Stmt* stmt, int& offset) {
    if (auto* b = dynamic_cast<BlockStmt*>(stmt)) {
        scanDecls(b, offset);
    } else if (auto* ifs = dynamic_cast<IfStmt*>(stmt)) {
        scanStmt(ifs->thenBranch.get(), offset);
        if (ifs->elseBranch)
            scanStmt(ifs->elseBranch.get(), offset);
    } else if (auto* ws = dynamic_cast<WhileStmt*>(stmt)) {
        scanStmt(ws->body.get(), offset);
    } else if (auto* ds = dynamic_cast<DoWhileStmt*>(stmt)) {
        scanStmt(ds->body.get(), offset);
    } else if (auto* sw = dynamic_cast<SwitchStmt*>(stmt)) {
        for (auto& clause : sw->clauses)
            for (auto& body : clause.statements)
                scanStmt(body.get(), offset);
    }
}

// ============================================================
// 语句生成
// ============================================================
void CodeGen::genBlock(BlockStmt* block) {
    pushScope();
    for (auto& item : block->items) {
        if (item.kind == BlockStmt::Item::Kind::Decl)
            genDecl(item.decl.get());
        else
            genStmt(item.stmt.get());
    }
    popScope();
}

void CodeGen::genDecl(DeclStmt* decl) {
    for (auto& def : decl->defs) {
        Symbol sym;
        sym.name      = decl->isStatic ? "__sysy_static_" + curFuncName_ +
                                         "_" + def->name : def->name;
        sym.type      = def->type;
        sym.type.base = decl->baseType;
        sym.isConst   = decl->isConst;
        sym.isGlobal  = decl->isStatic;

        if (decl->isStatic) {
            const int total = def->type.isArray() ? def->type.totalElems() : 1;
            const int bytes = total * 4;
            if (!def->initVal) {
                out_ << "\n  .bss\n  .align 2\n" << sym.name << ":\n";
                out_ << "  .zero " << bytes << "\n  .text\n";
            } else {
                out_ << "\n  .data\n  .align 2\n" << sym.name << ":\n";
                int index = 0;
                emitGlobalData(sym, def->initVal.get(), index, total, 0);
                while (index < total) { out_ << "  .word 0\n"; ++index; }
                out_ << "  .text\n";
            }
            addSym(sym);
            scopes_.back()[def->name] = sym;
            continue;
        }

        // const 标量：编译期内联
        if (decl->isConst && !def->type.isArray()) {
            auto* sc = dynamic_cast<ScalarInit*>(def->initVal.get());
            if (decl->baseType == BaseType::Int)
                sym.constInt = evalConstInt(sc->expr.get());
            else
                sym.constFloat = evalConstFloat(sc->expr.get());
            sym.fpOffset = 0;
            addSym(sym);
            continue;
        }

        // 分配栈槽
        sym.fpOffset = allocLocal(slotSizeForType(sym.type),
                                  slotAlignForType(sym.type));
        addSym(sym);

        // 生成初始化
        if (def->initVal)
            genLocalInit(sym, def->initVal.get());
    }
}

// 局部变量初始化
void CodeGen::genLocalInit(Symbol& sym, InitVal* init) {
    if (!sym.type.isArray()) {
        auto* sc = dynamic_cast<ScalarInit*>(init);
        BaseType rt = genExpr(sc->expr.get());
        if (sym.type.base == BaseType::Float) {
            if (rt == BaseType::Int) intToFloat();
            emitFsw("fa0", "s0", sym.fpOffset);
        } else {
            if (rt == BaseType::Float) floatToInt();
            emitSw("a0", "s0", sym.fpOffset);
        }
        return;
    }
    // 数组：先全部清零（用循环避免生成海量指令）
    int total = sym.type.totalElems();
    if (total <= 8) {
        // 小数组：直接展开
        for (int i = 0; i < total; ++i)
            emitSw("zero", "s0", sym.fpOffset + i * 4);
    } else {
        // 大数组：用汇编循环 memset
        // t4 = 起始地址，t5 = 结束地址
        emitAddrAdd("t4", "s0", sym.fpOffset);
        loadImm("t5", (int64_t)total * 4);
        emit("add  t5, t4, t5");
        std::string loopL = newLabel(".Lzero_");
        emitLabel(loopL);
        emit("sw   zero, 0(t4)");
        emit("addi t4, t4, 4");
        emit("blt  t4, t5, " + loopL);
    }

    // 然后按初始化列表填充
    if (auto* agg = dynamic_cast<AggregateInit*>(init)) {
        int idx = 0;
        genArrayInit(sym, agg, idx, 0);
    } else if (auto* str = dynamic_cast<StringInit*>(init)) {
        int idx = 0;
        for (unsigned char c : str->value) {
            if (idx >= total) break;
            loadImm("a0", c);
            emitSw("a0", "s0", sym.fpOffset + idx * 4);
            ++idx;
        }
        if (idx < total) emitSw("zero", "s0", sym.fpOffset + idx * 4);
    }
}

void CodeGen::genArrayInit(Symbol& sym, AggregateInit* agg,
                            int& elemIdx, int depth) {
    // 计算当前层的子数组大小：dims[depth+1] * dims[depth+2] * ...
    const auto& dims = sym.type.dims;
    int subSize = 1;
    for (int d = depth + 1; d < (int)dims.size(); ++d)
        if (dims[d] > 0) subSize *= dims[d];

    for (auto& item : agg->items) {
        if (auto* nested = dynamic_cast<AggregateInit*>(item.get())) {
            // 嵌套大括号：先将 elemIdx 对齐到子数组边界
            if (subSize > 1 && elemIdx % subSize != 0)
                elemIdx = (elemIdx / subSize + 1) * subSize;
            int baseIdx = elemIdx;
            genArrayInit(sym, nested, elemIdx, depth + 1);
            // 处理完后，推进到本子数组末尾（跳过未初始化的尾部元素）
            if (subSize > 1)
                elemIdx = baseIdx / subSize * subSize + subSize;
        } else if (auto* sc = dynamic_cast<ScalarInit*>(item.get())) {
            int byteOff = sym.fpOffset + elemIdx * 4;
            BaseType rt = genExpr(sc->expr.get());
            if (sym.type.base == BaseType::Float) {
                if (rt == BaseType::Int) intToFloat();
                emitFsw("fa0", "s0", byteOff);
            } else {
                if (rt == BaseType::Float) floatToInt();
                emitSw("a0", "s0", byteOff);
            }
            ++elemIdx;
        }
    }
}

void CodeGen::genStmt(Stmt* s) {
    // ---- 块 ----
    if (auto* b = dynamic_cast<BlockStmt*>(s)) {
        genBlock(b); return;
    }
    // ---- 赋值 ----
    if (auto* as = dynamic_cast<AssignStmt*>(s)) {
        // 1. 求右值 → a0 或 fa0
        BaseType rtype = genExpr(as->rhs.get());
        // 2. 临时保存右值
        if (rtype == BaseType::Float) {
            emit("addi sp, sp, -8"); emit("fsw  fa0, 4(sp)");
        } else {
            emit("addi sp, sp, -8"); emit("sd   a0, 0(sp)");
        }
        // 3. 计算左值目标地址 → a0（或直接得到 fpOffset）
        auto* lv = dynamic_cast<LValExpr*>(as->lval.get());
        Symbol* sym = lookupSym(lv->name);
        if (!sym) throw std::runtime_error("undefined: " + lv->name);

        bool hasIdx = !lv->indices.empty();
        if (hasIdx) {
            genLValAddr(lv); // → a0
            emit("mv   t2, a0");   // 保存目标地址
        }
        // 4. 恢复右值，类型转换后写入
        if (rtype == BaseType::Float) {
            emit("flw  ft1, 4(sp)"); emit("addi sp, sp, 8");
            if (hasIdx) {
                if (sym->type.base == BaseType::Float)
                    emit("fsw  ft1, 0(t2)");
                else {
                    emit("fcvt.w.s a0, ft1, rtz");
                    emit("sw   a0, 0(t2)");
                }
            } else {
                if (sym->isGlobal) {
                    emitLoadGlobalAddr("t2", sym->name);
                    if (sym->type.base == BaseType::Float)
                        emit("fsw  ft1, 0(t2)");
                    else {
                        emit("fcvt.w.s a0, ft1, rtz");
                        emit("sw   a0, 0(t2)");
                    }
                } else {
                    if (sym->type.base == BaseType::Float) {
                        emitFsw("ft1", "s0", sym->fpOffset);
                    } else {
                        emit("fcvt.w.s a0, ft1, rtz");
                        emitSw("a0", "s0", sym->fpOffset);
                    }
                }
            }
        } else {
            // int 右值
            emit("ld   t1, 0(sp)"); emit("addi sp, sp, 8");
            if (hasIdx) {
                if (sym->type.base == BaseType::Float) {
                    emit("fcvt.s.w fa0, t1");
                    emit("fsw  fa0, 0(t2)");
                } else {
                    emit("sw   t1, 0(t2)");
                }
            } else {
                if (sym->isGlobal) {
                    emitLoadGlobalAddr("t2", sym->name);
                    if (sym->type.base == BaseType::Float) {
                        emit("fcvt.s.w fa0, t1");
                        emit("fsw  fa0, 0(t2)");
                    } else {
                        emit("sw   t1, 0(t2)");
                    }
                } else {
                    if (sym->type.base == BaseType::Float) {
                        emit("fcvt.s.w fa0, t1");
                        emitFsw("fa0", "s0", sym->fpOffset);
                    } else {
                        emitSw("t1", "s0", sym->fpOffset);
                    }
                }
            }
        }
        return;
    }
    // ---- 表达式语句 ----
    if (auto* es = dynamic_cast<ExprStmt*>(s)) {
        if (es->expr) genExpr(es->expr.get()); return;
    }
    // ---- if 语句 ----
    if (auto* ifs = dynamic_cast<IfStmt*>(s)) {
        std::string elseL = newLabel(".Lelse");
        std::string endL  = newLabel(".Lend_if");
        BaseType condTy = genExpr(ifs->cond.get());
        normalizeCond(condTy);
        emit("beqz a0, " + elseL);
        genStmt(ifs->thenBranch.get());
        if (ifs->elseBranch) emit("j    " + endL);
        emitLabel(elseL);
        if (ifs->elseBranch) { genStmt(ifs->elseBranch.get()); emitLabel(endL); }
        return;
    }
    // ---- while 循环 ----
    if (auto* ws = dynamic_cast<WhileStmt*>(s)) {
        std::string condL = newLabel(".Lwcond");
        std::string endL  = newLabel(".Lwend");
        auto sb = breakLabel_; auto sc = continueLabel_;
        breakLabel_ = endL; continueLabel_ = condL;
        emitLabel(condL);
        BaseType condTy = genExpr(ws->cond.get());
        normalizeCond(condTy);
        emit("beqz a0, " + endL);
        genStmt(ws->body.get());
        emit("j    " + condL);
        emitLabel(endL);
        breakLabel_ = sb; continueLabel_ = sc;
        return;
    }
    // ---- do-while 循环 ----
    if (auto* ds = dynamic_cast<DoWhileStmt*>(s)) {
        std::string bodyL = newLabel(".Ldo_body");
        std::string condL = newLabel(".Ldo_cond");
        std::string endL  = newLabel(".Ldo_end");
        auto sb = breakLabel_; auto sc = continueLabel_;
        breakLabel_ = endL; continueLabel_ = condL;
        emitLabel(bodyL);
        genStmt(ds->body.get());
        emitLabel(condL);
        BaseType condTy = genExpr(ds->cond.get());
        normalizeCond(condTy);
        emit("bnez a0, " + bodyL);
        emitLabel(endL);
        breakLabel_ = sb; continueLabel_ = sc;
        return;
    }
    if (auto* sw = dynamic_cast<SwitchStmt*>(s)) {
        const std::string endL = newLabel(".Lswitch_end");
        std::vector<std::string> clauseLabels;
        std::string defaultLabel = endL;
        for (const auto& clause : sw->clauses) {
            clauseLabels.push_back(newLabel(".Lswitch_case"));
            if (clause.isDefault) defaultLabel = clauseLabels.back();
        }
        BaseType valueType = genExpr(sw->value.get());
        if (valueType == BaseType::Float)
            throw std::runtime_error("switch expression must be integer");
        emit("mv   t6, a0");
        for (size_t i = 0; i < sw->clauses.size(); ++i) {
            const auto& clause = sw->clauses[i];
            if (clause.isDefault) continue;
            auto* intLit = dynamic_cast<IntLitExpr*>(clause.value.get());
            auto* charLit = dynamic_cast<CharLitExpr*>(clause.value.get());
            int value = intLit ? intLit->val : charLit ? charLit->val : 0;
            loadImm("t5", value);
            emit("beq  t6, t5, " + clauseLabels[i]);
        }
        emit("j    " + defaultLabel);
        auto oldBreak = breakLabel_;
        breakLabel_ = endL;
        for (size_t i = 0; i < sw->clauses.size(); ++i) {
            emitLabel(clauseLabels[i]);
            for (auto& body : sw->clauses[i].statements)
                genStmt(body.get());
            if (i + 1 == sw->clauses.size()) emit("j    " + endL);
        }
        breakLabel_ = oldBreak;
        emitLabel(endL);
        return;
    }
    if (auto* ps = dynamic_cast<PrintfStmt*>(s)) {
        size_t arg = 0;
        for (size_t i = 0; i < ps->format.size(); ++i) {
            if (ps->format[i] == '%' && i + 1 < ps->format.size()) {
                const char spec = ps->format[++i];
                if (arg >= ps->args.size()) break;
                auto call = std::make_unique<CallExpr>();
                call->line = ps->line;
                if (spec == 's') {
                    genExpr(ps->args[arg++].get());
                    emit("addi sp, sp, -8");
                    emit("sd   a0, 0(sp)");
                    std::string loopL = newLabel(".Lprintf_str");
                    std::string endL = newLabel(".Lprintf_str_end");
                    emitLabel(loopL);
                    emit("ld   t4, 0(sp)");
                    emit("lw   a0, 0(t4)");
                    emit("beqz a0, " + endL);
                    emit("call putch");
                    emit("ld   t4, 0(sp)");
                    emit("addi t4, t4, 4");
                    emit("sd   t4, 0(sp)");
                    emit("j    " + loopL);
                    emitLabel(endL);
                    emit("addi sp, sp, 8");
                    continue;
                }
                call->name = spec == 'd' ? "putint" : "putch";
                call->args.push_back(std::move(ps->args[arg++]));
                genCall(call.get());
            } else {
                auto call = std::make_unique<CallExpr>();
                call->line = ps->line;
                call->name = "putch";
                auto lit = std::make_unique<CharLitExpr>();
                lit->line = ps->line;
                lit->val = static_cast<uint8_t>(ps->format[i]);
                call->args.push_back(std::move(lit));
                genCall(call.get());
            }
        }
        return;
    }
    // ---- break / continue ----
    if (dynamic_cast<BreakStmt*>(s)) {
        if (breakLabel_.empty()) throw std::runtime_error("break outside loop");
        emit("j    " + breakLabel_); return;
    }
    if (dynamic_cast<ContinueStmt*>(s)) {
        if (continueLabel_.empty()) throw std::runtime_error("continue outside loop");
        emit("j    " + continueLabel_); return;
    }
    // ---- return 语句 ----
    if (auto* ret = dynamic_cast<ReturnStmt*>(s)) {
        if (ret->val) {
            BaseType rt = genExpr(ret->val.get());
            if (curRetType_.base == BaseType::Float && rt == BaseType::Int) intToFloat();
            else if (curRetType_.base == BaseType::Int && rt == BaseType::Float) floatToInt();
            else if (curRetType_.base == BaseType::Char && rt != BaseType::Float)
                emit("andi a0, a0, 255");
        }
        // 跳转到统一的函数 epilogue
        emit("j    " + curRetLabel_);
        return;
    }
    throw std::runtime_error("unknown statement type");
}

// ============================================================
// 表达式生成（结果: Int→a0, Float→fa0）
// ============================================================
BaseType CodeGen::genExpr(Expr* e) {
    if (auto* x = dynamic_cast<IntLitExpr*>(e)) {
        loadImm("a0", x->val); return BaseType::Int;
    }
    if (auto* x = dynamic_cast<FloatLitExpr*>(e)) {
        loadFloatConst(x->val); return BaseType::Float;
    }
    if (auto* x = dynamic_cast<CharLitExpr*>(e)) {
        loadImm("a0", x->val); return BaseType::Char;
    }
    if (auto* x = dynamic_cast<StringLitExpr*>(e)) {
        if (x->indices.size() != 1)
            throw std::runtime_error("string literal requires one constant index");
        int32_t index = evalConstInt(x->indices.front().get());
        if (index < 0 || index >= static_cast<int32_t>(x->value.size()))
            throw std::runtime_error("string literal index out of range");
        loadImm("a0", static_cast<unsigned char>(x->value[index]));
        return BaseType::Char;
    }
    if (auto* x = dynamic_cast<CastExpr*>(e)) {
        BaseType source = genExpr(x->operand.get());
        if (x->targetType.base == BaseType::Char) {
            if (source == BaseType::Float) floatToInt();
            emit("andi a0, a0, 255");
            return BaseType::Char;
        }
        if (x->targetType.base == BaseType::Int && source == BaseType::Float)
            floatToInt();
        return BaseType::Int;
    }
    if (auto* x = dynamic_cast<LValExpr*>(e))  return genLValLoad(x);
    if (auto* x = dynamic_cast<CallExpr*>(e))  return genCall(x);
    if (auto* x = dynamic_cast<UnaryExpr*>(e)) return genUnary(x);
    if (auto* x = dynamic_cast<BinaryExpr*>(e))return genBinary(x);
    throw std::runtime_error("unknown expr");
}

// ============================================================
// 二元表达式（先求左值压栈，再求右值，弹栈合并）
// ============================================================
BaseType CodeGen::genBinary(BinaryExpr* e) {
    if (e->op == "&&") return genLogicalAnd(e);
    if (e->op == "||") return genLogicalOr(e);

    // 求左值，将结果保存到运行时栈
    BaseType ltype = genExpr(e->lhs.get());
    if (ltype == BaseType::Float) {
        emit("addi sp, sp, -8"); emit("fsw  fa0, 4(sp)");
    } else {
        emit("addi sp, sp, -8"); emit("sd   a0, 0(sp)");
    }

    // 求右值（结果在 a0/fa0）
    BaseType rtype = genExpr(e->rhs.get());
    bool isFloat = (ltype == BaseType::Float || rtype == BaseType::Float);

    if (isFloat) {
        // 右值确保在 fa0
        if (rtype == BaseType::Int) intToFloat();
        // 从栈恢复左值到 fa1
        if (ltype == BaseType::Float) {
            emit("flw  fa1, 4(sp)");
        } else {
            emit("lw   a1, 0(sp)");
            emit("fcvt.s.w fa1, a1");
        }
        emit("addi sp, sp, 8");
        // 浮点运算
        if (e->op=="+"||e->op=="-"||e->op=="*"||e->op=="/") {
            emitFloatBinop(e->op); return BaseType::Float;
        }
        emitFloatCmp(e->op); return BaseType::Int;
    } else {
        // 整型运算
        emit("lw   a1, 0(sp)"); // 注意 lw 自动符号扩展
        emit("addi sp, sp, 8");
        if (e->op=="+"||e->op=="-"||e->op=="*"||e->op=="/"||e->op=="%"||
            e->op=="&"||e->op=="|"||e->op=="^") {
            emitIntBinop(e->op);
            if (e->semaType && e->semaType->base == BaseType::Char) {
                emit("andi a0, a0, 255");
                return BaseType::Char;
            }
            return BaseType::Int;
        }
        emitIntCmp(e->op); return BaseType::Int;
    }
}

// 短路 &&
BaseType CodeGen::genLogicalAnd(BinaryExpr* e) {
    std::string falseL = newLabel(".Land_f");
    std::string endL   = newLabel(".Land_e");
    BaseType lhsTy = genExpr(e->lhs.get());
    normalizeCond(lhsTy);
    emit("beqz a0, " + falseL);
    BaseType rhsTy = genExpr(e->rhs.get());
    normalizeCond(rhsTy);
    emit("beqz a0, " + falseL);
    emit("li   a0, 1");
    emit("j    " + endL);
    emitLabel(falseL);
    emit("li   a0, 0");
    emitLabel(endL);
    return BaseType::Int;
}

// 短路 ||
BaseType CodeGen::genLogicalOr(BinaryExpr* e) {
    std::string trueL = newLabel(".Lor_t");
    std::string endL  = newLabel(".Lor_e");
    BaseType lhsTy = genExpr(e->lhs.get());
    normalizeCond(lhsTy);
    emit("bnez a0, " + trueL);
    BaseType rhsTy = genExpr(e->rhs.get());
    normalizeCond(rhsTy);
    emit("bnez a0, " + trueL);
    emit("li   a0, 0");
    emit("j    " + endL);
    emitLabel(trueL);
    emit("li   a0, 1");
    emitLabel(endL);
    return BaseType::Int;
}

// ============================================================
// 一元表达式
// ============================================================
BaseType CodeGen::genUnary(UnaryExpr* e) {
    BaseType t = genExpr(e->operand.get());
    if (e->op == '+') return t;
    if (e->op == '-') {
        if (t == BaseType::Float) { emit("fneg.s fa0, fa0"); }
        else {
            emit("negw  a0, a0");
            if (t == BaseType::Char) emit("andi a0, a0, 255");
        }
        return t;
    }
    if (e->op == '!') {
        normalizeCond(t);
        emit("xori a0, a0, 1");
        return BaseType::Int;
    }
    if (e->op == '~') {
        emit("xori a0, a0, -1");
        return BaseType::Int;
    }
    return t;
}

// ============================================================
// 左值地址计算 → a0（64 位地址）
// ============================================================
BaseType CodeGen::genLValAddr(LValExpr* lv) {
    Symbol* sym = lookupSym(lv->name);
    if (!sym) throw std::runtime_error("undefined: " + lv->name);

    // 无下标：取变量自身地址（数组基地址或标量地址）
    if (lv->indices.empty()) {
        if (sym->isGlobal)
            emitLoadGlobalAddr("a0", sym->name);
        else if (sym->isParam && sym->type.isArray())
            emitLd("a0", "s0", sym->fpOffset);
        else
            emitAddrAdd("a0", "s0", sym->fpOffset);
        return sym->type.base;
    }

    // 有下标：计算 base + index * stride（字节）
    // 先获取基地址
    if (sym->isGlobal)
        emitLoadGlobalAddr("a0", sym->name);
    else if (sym->isParam && sym->type.isArray())
        emitLd("a0", "s0", sym->fpOffset);
    else
        emitAddrAdd("a0", "s0", sym->fpOffset);

    // 逐维计算偏移，累加到 a0
    const auto& dims = sym->type.dims;
    int ndim = (int)lv->indices.size();

    for (int d = 0; d < ndim; ++d) {
        // 步长 = 后续各维大小之积 * 4（字节）
        int stride = 4;
        for (int k = d+1; k < (int)dims.size(); ++k) {
            if (dims[k] > 0) stride *= dims[k];
        }
        // 参数首维（-1）不参与步长计算

        // 保存当前基地址
        emit("addi sp, sp, -8"); emit("sd   a0, 0(sp)");

        // 求第 d 维下标 → a0（32 位 int，需要符号扩展到 64 位）
        genExpr(lv->indices[d].get());
        // a0 是 32 位下标（lw 已符号扩展），乘以步长
        if (stride != 1) {
            loadImm("t0", stride);
            emit("mul  a0, a0, t0"); // 64 位乘（下标已 sext）
        }

        // 恢复基地址，累加
        emit("ld   t1, 0(sp)"); emit("addi sp, sp, 8");
        emit("add  a0, t1, a0");
    }
    return sym->type.base;
}

// ============================================================
// 左值读取（load）
// ============================================================
BaseType CodeGen::genLValLoad(LValExpr* lv) {
    Symbol* sym = lookupSym(lv->name);
    if (!sym) throw std::runtime_error("undefined: " + lv->name);

    // const 标量内联
    if (sym->isConst && lv->indices.empty()) {
        if (sym->constInt.has_value())    { loadImm("a0", *sym->constInt); return BaseType::Int; }
        if (sym->constFloat.has_value())  { loadFloatConst(*sym->constFloat); return BaseType::Float; }
    }

    // 数组名（不带下标）→ 传地址
    if (lv->indices.empty() && sym->type.isArray()) {
        if (sym->isGlobal)
            emitLoadGlobalAddr("a0", sym->name);
        else if (sym->isParam && sym->type.isArray())
            emitLd("a0", "s0", sym->fpOffset);
        else
            emitAddrAdd("a0", "s0", sym->fpOffset);
        return BaseType::Int; // 地址视为 int
    }

    // 带下标：计算地址后 load
    if (!lv->indices.empty()) {
        genLValAddr(lv); // 地址 → a0
        // 如果下标数 < 维度数，表示部分下标访问，结果是子数组的指针，
        // 直接返回地址（不做 load），用于作为数组参数传递
        int ndims = (int)sym->type.dims.size();
        if ((int)lv->indices.size() < ndims) {
            return BaseType::Int; // 地址已在 a0，视为 int（指针）
        }
    if (sym->type.base == BaseType::Float) {
            emit("flw  fa0, 0(a0)"); return BaseType::Float;
        } else {
            emit("lw   a0, 0(a0)");
            if (sym->type.base == BaseType::Char) emit("andi a0, a0, 255");
            return sym->type.base;
        }
    }

    // 标量
    if (sym->isGlobal) {
        emitLoadGlobalAddr("t0", sym->name);
        if (sym->type.base == BaseType::Float) {
            emit("flw  fa0, 0(t0)"); return BaseType::Float;
        } else {
            emit("lw   a0, 0(t0)");
            if (sym->type.base == BaseType::Char) emit("andi a0, a0, 255");
            return sym->type.base;
        }
    } else {
        if (sym->type.base == BaseType::Float) {
            emitFlw("fa0", "s0", sym->fpOffset);
            return BaseType::Float;
        } else {
            emitLw("a0", "s0", sym->fpOffset);
            if (sym->type.base == BaseType::Char) emit("andi a0, a0, 255");
            return sym->type.base;
        }
    }
}

// ============================================================
// 函数调用（参数传递遵循 RV64GC lp64d ABI）
// ============================================================
BaseType CodeGen::genCall(CallExpr* e) {
    const bool userDefined = userFunctions_.count(e->name) != 0;
    // 特殊内置：starttime / stoptime
    if (!userDefined &&
        (e->name == "starttime" || e->name == "stoptime")) {
        std::string rname = (e->name == "starttime") ? "_sysy_starttime" : "_sysy_stoptime";
        loadImm("a0", e->line);
        emit("call " + rname);
        return BaseType::Void;
    }

    if (!userDefined && e->name == "getstr" && e->args.size() == 2) {
        // SysY2026 char arrays use the compiler's canonical word layout. Read
        // characters one at a time so the external byte-oriented runtime does
        // not write through that layout directly.
        genExpr(e->args[0].get());
        emit("addi sp, sp, -32");
        emit("sd   a0, 0(sp)");
        genExpr(e->args[1].get());
        emit("sd   a0, 8(sp)");
        emit("sd   zero, 16(sp)");
        std::string loopL = newLabel(".Lgetstr");
        std::string newlineL = newLabel(".Lgetstr_newline");
        std::string stopL = newLabel(".Lgetstr_stop");
        std::string doneL = newLabel(".Lgetstr_done");
        emit("ld   t4, 8(sp)");
        emit("blez t4, " + doneL);
        emitLabel(loopL);
        emit("ld   t5, 16(sp)");
        emit("ld   t4, 8(sp)");
        emit("addi t4, t4, -1");
        emit("bge  t5, t4, " + stopL);
        emit("call getch");
        emit("ld   t4, 0(sp)");
        emit("ld   t5, 16(sp)");
        emit("slli t6, t5, 2");
        emit("add  t4, t4, t6");
        emit("sw   a0, 0(t4)");
        emit("li   t4, 10");
        emit("beq  a0, t4, " + newlineL);
        emit("ld   t5, 16(sp)");
        emit("addi t5, t5, 1");
        emit("sd   t5, 16(sp)");
        emit("j    " + loopL);
        emitLabel(newlineL);
        emit("ld   t5, 16(sp)");
        emit("addi t5, t5, 1");
        emit("sd   t5, 16(sp)");
        emit("j    " + stopL);
        emitLabel(stopL);
        emit("ld   t4, 0(sp)");
        emit("ld   t5, 16(sp)");
        emit("slli t6, t5, 2");
        emit("add  t4, t4, t6");
        emit("sw   zero, 0(t4)");
        emit("j    " + doneL);
        emitLabel(doneL);
        emit("addi sp, sp, 32");
        return BaseType::Void;
    }

    if (!userDefined && e->name == "putstr" && e->args.size() == 1) {
        genExpr(e->args[0].get());
        emit("addi sp, sp, -16");
        emit("sd   a0, 0(sp)");
        std::string loopL = newLabel(".Lputstr");
        std::string endL = newLabel(".Lputstr_end");
        emitLabel(loopL);
        emit("ld   t4, 0(sp)");
        emit("lw   a0, 0(t4)");
        emit("beqz a0, " + endL);
        emit("call putch");
        emit("ld   t4, 0(sp)");
        emit("addi t4, t4, 4");
        emit("sd   t4, 0(sp)");
        emit("j    " + loopL);
        emitLabel(endL);
        emit("addi sp, sp, 16");
        return BaseType::Void;
    }

    int nArgs = (int)e->args.size();
    int alignPad = (nArgs % 2 == 0) ? 0 : 8;
    if (alignPad > 0)
        emit("addi sp, sp, -" + std::to_string(alignPad));

    // ---- 阶段1：逆序求值所有参数，压入临时参数区 ----
    // 逆序求值使得 arg0 位于 sp+0，arg i 位于 sp+i*8。
    std::vector<std::pair<BaseType, bool>> argTypes(nArgs);
    auto paramIt = funcParamTypes_.find(e->name);
    bool hasParamInfo = (paramIt != funcParamTypes_.end());
    for (int i = nArgs - 1; i >= 0; --i) {
        BaseType t = genExpr(e->args[i].get());
        bool isArrayParam = false;
        if (hasParamInfo && i < (int)paramIt->second.size()) {
            BaseType expected = paramIt->second[i].first;
            isArrayParam = paramIt->second[i].second;
            if (!isArrayParam && t != expected) {
                if (expected == BaseType::Int && t == BaseType::Float) floatToInt();
                else if (expected == BaseType::Float && t == BaseType::Int) intToFloat();
                else if (expected == BaseType::Char && t == BaseType::Float) floatToInt();
                t = expected;
            }
            if (!isArrayParam && expected == BaseType::Char && t != BaseType::Float) {
                emit("andi a0, a0, 255");
                t = BaseType::Char;
            }
        }
        argTypes[i] = {t, isArrayParam};
        if (t == BaseType::Float) {
            emit("addi sp, sp, -8"); emit("fsw  fa0, 0(sp)");
        } else {
            emit("addi sp, sp, -8"); emit("sd   a0, 0(sp)");
        }
    }

    AbiArgLayout argLayout = assignAbiArgLayout(argTypes);

    // 阶段2：装载寄存器参数。
    for (int i = 0; i < nArgs; ++i) {
        const AbiArgLoc& loc = argLayout.locs[i];
        if (loc.kind == AbiArgLocKind::Stack) continue;
        int srcOff = i * 8;
        if (loc.kind == AbiArgLocKind::FloatReg)
            emitFlw("fa" + std::to_string(loc.index), "sp", srcOff);
        else if (loc.isFloat)
            emitLw("a" + std::to_string(loc.index), "sp", srcOff);
        else
            emitLd("a" + std::to_string(loc.index), "sp", srcOff);
    }

    // 阶段3：把真正的栈参数压缩到 sp+0, sp+8, ...
    for (int i = 0; i < nArgs; ++i) {
        const AbiArgLoc& loc = argLayout.locs[i];
        if (loc.kind != AbiArgLocKind::Stack) continue;
        int srcOff = i * 8;
        int dstOff = loc.index * 8;
        if (loc.isFloat) {
            emitLw("t0", "sp", srcOff);
            emitSw("t0", "sp", dstOff);
        } else {
            emitLd("t0", "sp", srcOff);
            emitSd("t0", "sp", dstOff);
        }
    }

    emit("call " + e->name);

    // 清理整个临时参数区和对齐填充。
    int cleanupBytes = nArgs * 8 + alignPad;
    if (cleanupBytes > 0)
        emitAddrAdd("sp", "sp", cleanupBytes);

    // 确定返回类型
    // 对内置运行时函数：硬编码
    static const std::unordered_map<std::string, BaseType> BUILTINS = {
        {"getint",    BaseType::Int},   {"getch",     BaseType::Int},
        {"getarray",  BaseType::Int},   {"putint",    BaseType::Void},
        {"putch",     BaseType::Void},  {"putarray",  BaseType::Void},
        {"getfloat",  BaseType::Float}, {"getfarray", BaseType::Int},
        {"putfloat",  BaseType::Void},  {"putfarray", BaseType::Void},
        {"getstr", BaseType::Void}, {"putstr", BaseType::Void},
    };
    // 用户函数：从预扫描的函数返回类型表中查询
    auto uit = funcRetTypes_.find(e->name);
    if (userDefined && uit != funcRetTypes_.end()) return uit->second;

    auto it = BUILTINS.find(e->name);
    if (it != BUILTINS.end()) return it->second;

    if (uit != funcRetTypes_.end()) return uit->second;
    return BaseType::Int; // 未知函数默认 int
}
