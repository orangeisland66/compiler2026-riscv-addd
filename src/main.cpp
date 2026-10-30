// 文件职责：编译器命令行入口，串接前端、HIR/AST 重写、CoreIR 优化和后端输出。本文件负责实现具体逻辑。
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "analysis/analysis_manager.h"
#include "core_ir/ir/core_ir.h"
#include "core_ir/lowering/ast_to_ir.h"
#include "core_ir/passes/passes.h"
#include "core_ir/passes/pipeline_builder.h"
#include "hir/ast_simplify.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "hir/cross_call_array_summary.h"
#include "hir/demand_driven_copy_transform.h"
#include "hir/radix_sort_lowering.h"
#endif
#include "hir/reflective_max_simplify.h"
#include "hir/ast_matmul_interchange.h"
#include "hir/linear_recurrence_fusion.h"
#include "hir/recursive_accumulator_normalize.h"
#include "hir/tail_recursion.h"
#include "backend_mir/mir_backend.h"
#include "research_switch.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

struct Args {
  std::string input;
  std::string output;
  bool optimize = false;
};

static bool hasSysY2026Expr(const Expr* expr);

static bool hasSysY2026Init(const InitVal* init) {
  if (!init) return false;
  if (dynamic_cast<const StringInit*>(init)) return true;
  if (auto* scalar = dynamic_cast<const ScalarInit*>(init))
    return hasSysY2026Expr(scalar->expr.get());
  if (auto* aggregate = dynamic_cast<const AggregateInit*>(init))
    for (const auto& item : aggregate->items)
      if (hasSysY2026Init(item.get())) return true;
  return false;
}

static bool hasSysY2026Expr(const Expr* expr) {
  if (!expr) return false;
  if (dynamic_cast<const CharLitExpr*>(expr) ||
      dynamic_cast<const StringLitExpr*>(expr) ||
      dynamic_cast<const CastExpr*>(expr)) return true;
  if (auto* lv = dynamic_cast<const LValExpr*>(expr))
    for (const auto& index : lv->indices)
      if (hasSysY2026Expr(index.get())) return true;
  if (auto* call = dynamic_cast<const CallExpr*>(expr))
    for (const auto& arg : call->args)
      if (hasSysY2026Expr(arg.get())) return true;
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return hasSysY2026Expr(unary->operand.get());
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return hasSysY2026Expr(binary->lhs.get()) ||
           hasSysY2026Expr(binary->rhs.get());
  return false;
}

static bool hasSysY2026Stmt(const Stmt* stmt);

static bool hasSysY2026Decl(const DeclStmt* decl) {
  if (!decl) return false;
  if (decl->isStatic || decl->baseType == BaseType::Char) return true;
  for (const auto& def : decl->defs)
    if (hasSysY2026Init(def->initVal.get())) return true;
  return false;
}

static bool hasSysY2026Stmt(const Stmt* stmt) {
  if (!stmt) return false;
  if (dynamic_cast<const SwitchStmt*>(stmt) ||
      dynamic_cast<const PrintfStmt*>(stmt)) return true;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl &&
          hasSysY2026Decl(item.decl.get())) return true;
      if (item.kind == BlockStmt::Item::Kind::Stmt &&
          hasSysY2026Stmt(item.stmt.get())) return true;
    }
  } else if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    return hasSysY2026Expr(assign->lval.get()) ||
           hasSysY2026Expr(assign->rhs.get());
  } else if (auto* expr = dynamic_cast<const ExprStmt*>(stmt)) {
    return hasSysY2026Expr(expr->expr.get());
  } else if (auto* ifs = dynamic_cast<const IfStmt*>(stmt)) {
    return hasSysY2026Expr(ifs->cond.get()) ||
           hasSysY2026Stmt(ifs->thenBranch.get()) ||
           hasSysY2026Stmt(ifs->elseBranch.get());
  } else if (auto* loop = dynamic_cast<const WhileStmt*>(stmt)) {
    return hasSysY2026Expr(loop->cond.get()) ||
           hasSysY2026Stmt(loop->body.get());
  } else if (auto* loop = dynamic_cast<const DoWhileStmt*>(stmt)) {
    return hasSysY2026Expr(loop->cond.get()) ||
           hasSysY2026Stmt(loop->body.get());
  } else if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    return hasSysY2026Expr(ret->val.get());
  }
  return false;
}

static bool hasSysY2026Features(const CompUnit* unit) {
  for (const auto& item : unit->items) {
    if (item.kind == CompUnit::Item::Kind::GlobalDecl) {
      if (hasSysY2026Decl(item.decl.get())) return true;
    } else {
      if (item.func->retType.base == BaseType::Char) return true;
      for (const auto& param : item.func->params)
        if (param->type.base == BaseType::Char) return true;
      if (hasSysY2026Stmt(item.func->body.get())) return true;
    }
  }
  return false;
}

static Args parseArgs(int argc, char* argv[]) {
  Args args;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "-S") {
      continue;
    }
    if (a == "-o") {
      if (i + 1 >= argc)
        throw std::runtime_error("-o requires output filename");
      args.output = argv[++i];
      continue;
    }
    if (a == "-O1") {
      args.optimize = true;
      continue;
    }
    if (a == "-O2" || a == "-O3" || a == "--no-core-ir-opt" ||
        a == "--legacy-backend" || a == "--mir-backend")
      throw std::runtime_error("unsupported CLI option: " + a);
    if (!a.empty() && a[0] != '-') {
      args.input = a;
    }
  }
  if (args.input.empty())
    throw std::runtime_error("no input file specified");
  return args;
}

int main(int argc, char* argv[]) {
  try {
    Args args = parseArgs(argc, argv);

    std::ifstream inFile(args.input);
    if (!inFile.is_open())
      throw std::runtime_error("cannot open input file: " + args.input);
    std::string source((std::istreambuf_iterator<char>(inFile)),
                       std::istreambuf_iterator<char>());
    Lexer lexer(source, args.input);
    Parser parser(lexer);
    auto cu = parser.parseCompUnit();

    Sema sema;
    auto diagnostics = sema.analyze(cu.get());
    for (const auto& diagnostic : lexer.diagnostics())
      diagnostics.push_back({diagnostic.line, diagnostic.code});
    for (const auto& diagnostic : parser.diagnostics())
      diagnostics.push_back({diagnostic.line, diagnostic.code});
    std::sort(diagnostics.begin(), diagnostics.end(),
              [](const auto& lhs, const auto& rhs) {
                if (lhs.line != rhs.line) return lhs.line < rhs.line;
                return lhs.code < rhs.code;
              });
    diagnostics.erase(
        std::unique(diagnostics.begin(), diagnostics.end(),
                    [](const auto& lhs, const auto& rhs) {
                      return lhs.line == rhs.line && lhs.code == rhs.code;
                    }),
        diagnostics.end());
    if (!diagnostics.empty()) {
      for (const auto& diagnostic : diagnostics)
        std::cerr << diagnostic.line << " " << diagnostic.code << "\n";
      return 1;
    }
    const bool hasExtendedLanguage = hasSysY2026Features(cu.get());

    std::ostream* outStream = &std::cout;
    std::ofstream outFile;
    if (!args.output.empty()) {
      outFile.open(args.output);
      if (!outFile.is_open())
        throw std::runtime_error("cannot open output file: " + args.output);
      outStream = &outFile;
    }

    if (args.optimize) {
      if (!hasExtendedLanguage && !sycEnvironmentValue("SYC_SKIP_AST_SIMPLIFY")) {
        bool astSimplified = hir::simplifyAST(*cu);
        (void)astSimplified;
      }
#ifdef SYC_ENABLE_RESEARCH_PASSES
      bool astMatmulInterchanged = false;
      if (!hasExtendedLanguage &&
          sycResearchSwitchEnabled("SYC_ENABLE_AST_MATMUL_INTERCHANGE")) {
        astMatmulInterchanged = hir::applyASTMatmulInterchange(*cu);
        if (astMatmulInterchanged &&
            sycEnvironmentValue("SYC_TRACE_AST_MATMUL_INTERCHANGE"))
          std::cerr << "[ast-matmul] applied\n";
      }
      (void)astMatmulInterchanged;

#endif
#ifdef SYC_ENABLE_RESEARCH_PASSES
      // These whole-algorithm AST rewrites remain outside submission builds
      // until their proof archives cover every accepted program. Research
      // execution requires both the global opt-in and the specific switch.
      if (!hasExtendedLanguage &&
          sycResearchSwitchEnabled("SYC_ENABLE_AST_RADIX_SORT_LOWERING"))
        (void)hir::lowerRecursiveRadixSort(*cu);
      if (!hasExtendedLanguage && sycResearchSwitchEnabled(
              "SYC_ENABLE_AST_DEMAND_DRIVEN_COPY_TRANSFORM"))
        (void)hir::applyDemandDrivenCopyTransform(*cu);
      if (!hasExtendedLanguage &&
          sycResearchSwitchEnabled("SYC_ENABLE_AST_CROSS_CALL_ARRAY_SUMMARY"))
        (void)hir::applyCrossCallArraySummary(*cu);
#endif
      // 在 AST 转 IR 前处理直接标量尾调用；否则共享的返回 PHI 会让 CoreIR
      // 阶段无法继续识别这类调用。
      bool eliminatedASTTailRecursion = false;
      if (!hasExtendedLanguage && !sycEnvironmentValue("SYC_SKIP_AST_TAIL_RECURSION"))
        eliminatedASTTailRecursion = hir::eliminateTailRecursion(*cu);
      (void)eliminatedASTTailRecursion;
      bool normalizedRecursiveAccumulators = false;
      if (!hasExtendedLanguage &&
          !sycEnvironmentValue("SYC_SKIP_AST_RECURSIVE_ACCUMULATOR_NORMALIZE"))
        normalizedRecursiveAccumulators =
            hir::normalizeRecursiveAccumulators(*cu);
      bool reflectiveMaxSimplified = false;
#ifdef SYC_ENABLE_RESEARCH_PASSES
      if (!hasExtendedLanguage &&
          sycResearchSwitchEnabled("SYC_ENABLE_AST_REFLECTIVE_MAX_SIMPLIFY"))
        reflectiveMaxSimplified = hir::simplifyReflectiveMaxLadders(*cu);
#endif
      (void)reflectiveMaxSimplified;
      bool linearRecurrenceFused = false;
#ifdef SYC_ENABLE_RESEARCH_PASSES
      if (!hasExtendedLanguage &&
          sycResearchSwitchEnabled("SYC_ENABLE_AST_LINEAR_RECURRENCE_FUSION"))
        linearRecurrenceFused = hir::applyLinearRecurrenceFusion(*cu);
#endif
      (void)linearRecurrenceFused;

      std::ostringstream irAsm;
      core_ir::ASTToIR astToIR;
      auto mod = astToIR.lower(cu.get());
      analysis::AnalysisManager analysisManager(*mod);
      if (sycEnvironmentValue("SYC_DUMP_ANALYSIS"))
        analysisManager.dump(std::cerr);
      core_ir::PipelineBuildOptions buildOptions;
      buildOptions.mode = core_ir::PipelineMode::Performance;
      buildOptions.coreIROptEnabled = !hasExtendedLanguage;
      buildOptions.hirSummary.preservesLoopMemoryStructure = true;
      buildOptions.hirSummary.rewroteRecursiveStructure =
          normalizedRecursiveAccumulators;
      buildOptions.analysisManager = &analysisManager;
      core_ir::PipelinePlan pipelinePlan =
          core_ir::buildPipelinePlan(buildOptions);
      if (pipelinePlan.runCoreIR && !sycEnvironmentValue("SYC_SKIP_COREIR_OPT"))
        core_ir::runCoreIROptimizations(*mod, pipelinePlan.coreIR);
      if (sycEnvironmentValue("COREIR_DUMP"))
        core_ir::printModule(std::cerr, *mod);

      // 性能入口统一走 MIR 后端。旧 CoreIR 直出 RISC-V 路径已移除，
      // 后续正确性和性能问题都必须在 MIR lowering / regalloc / cleanup 内闭环。
      mir::runMIRBackend(*mod, irAsm);
      *outStream << irAsm.str();
    } else {
      CodeGen cg(*outStream, false);
      cg.generate(cu.get());
    }

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }
}
  // 构建标记：2026-07-20，强制重新构建。
