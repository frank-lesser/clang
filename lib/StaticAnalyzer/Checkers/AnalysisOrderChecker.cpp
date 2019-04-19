//===- AnalysisOrderChecker - Print callbacks called ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This checker prints callbacks that are called during analysis.
// This is required to ensure that callbacks are fired in order
// and do not duplicate or get lost.
// Feel free to extend this checker with any callback you need to check.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Analysis/CFGStmtMap.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class AnalysisOrderChecker
    : public Checker<check::PreStmt<CastExpr>,
                     check::PostStmt<CastExpr>,
                     check::PreStmt<ArraySubscriptExpr>,
                     check::PostStmt<ArraySubscriptExpr>,
                     check::PreStmt<CXXNewExpr>,
                     check::PostStmt<CXXNewExpr>,
                     check::PreStmt<OffsetOfExpr>,
                     check::PostStmt<OffsetOfExpr>,
                     check::PreCall,
                     check::PostCall,
                     check::EndFunction,
                     check::NewAllocator,
                     check::Bind,
                     check::RegionChanges,
                     check::LiveSymbols> {

  bool isCallbackEnabled(AnalyzerOptions &Opts, StringRef CallbackName) const {
    return Opts.getCheckerBooleanOption(this, "*", false) ||
        Opts.getCheckerBooleanOption(this, CallbackName, false);
  }

  bool isCallbackEnabled(CheckerContext &C, StringRef CallbackName) const {
    AnalyzerOptions &Opts = C.getAnalysisManager().getAnalyzerOptions();
    return isCallbackEnabled(Opts, CallbackName);
  }

  bool isCallbackEnabled(ProgramStateRef State, StringRef CallbackName) const {
    AnalyzerOptions &Opts = State->getStateManager().getOwningEngine()
                                 .getAnalysisManager().getAnalyzerOptions();
    return isCallbackEnabled(Opts, CallbackName);
  }

public:
  void checkPreStmt(const CastExpr *CE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCastExpr"))
      llvm::errs() << "PreStmt<CastExpr> (Kind : " << CE->getCastKindName()
                   << ")\n";
  }

  void checkPostStmt(const CastExpr *CE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCastExpr"))
      llvm::errs() << "PostStmt<CastExpr> (Kind : " << CE->getCastKindName()
                   << ")\n";
  }

  void checkPreStmt(const ArraySubscriptExpr *SubExpr,
                    CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtArraySubscriptExpr"))
      llvm::errs() << "PreStmt<ArraySubscriptExpr>\n";
  }

  void checkPostStmt(const ArraySubscriptExpr *SubExpr,
                     CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtArraySubscriptExpr"))
      llvm::errs() << "PostStmt<ArraySubscriptExpr>\n";
  }

  void checkPreStmt(const CXXNewExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCXXNewExpr"))
      llvm::errs() << "PreStmt<CXXNewExpr>\n";
  }

  void checkPostStmt(const CXXNewExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCXXNewExpr"))
      llvm::errs() << "PostStmt<CXXNewExpr>\n";
  }

  void checkPreStmt(const OffsetOfExpr *OOE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtOffsetOfExpr"))
      llvm::errs() << "PreStmt<OffsetOfExpr>\n";
  }

  void checkPostStmt(const OffsetOfExpr *OOE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtOffsetOfExpr"))
      llvm::errs() << "PostStmt<OffsetOfExpr>\n";
  }

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreCall")) {
      llvm::errs() << "PreCall";
      if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Call.getDecl()))
        llvm::errs() << " (" << ND->getQualifiedNameAsString() << ')';
      llvm::errs() << '\n';
    }
  }

  void checkPostCall(const CallEvent &Call, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostCall")) {
      llvm::errs() << "PostCall";
      if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Call.getDecl()))
        llvm::errs() << " (" << ND->getQualifiedNameAsString() << ')';
      llvm::errs() << '\n';
    }
  }

  void checkEndFunction(const ReturnStmt *S, CheckerContext &C) const {
    if (isCallbackEnabled(C, "EndFunction")) {
      llvm::errs() << "EndFunction\nReturnStmt: " << (S ? "yes" : "no") << "\n";
      if (!S)
        return;

      llvm::errs() << "CFGElement: ";
      CFGStmtMap *Map = C.getCurrentAnalysisDeclContext()->getCFGStmtMap();
      CFGElement LastElement = Map->getBlock(S)->back();

      if (LastElement.getAs<CFGStmt>())
        llvm::errs() << "CFGStmt\n";
      else if (LastElement.getAs<CFGAutomaticObjDtor>())
        llvm::errs() << "CFGAutomaticObjDtor\n";
    }
  }

  void checkNewAllocator(const CXXNewExpr *CNE, SVal Target,
                         CheckerContext &C) const {
    if (isCallbackEnabled(C, "NewAllocator"))
      llvm::errs() << "NewAllocator\n";
  }

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &C) const {
    if (isCallbackEnabled(C, "Bind"))
      llvm::errs() << "Bind\n";
  }

  void checkLiveSymbols(ProgramStateRef State, SymbolReaper &SymReaper) const {
    if (isCallbackEnabled(State, "LiveSymbols"))
      llvm::errs() << "LiveSymbols\n";
  }

  ProgramStateRef
  checkRegionChanges(ProgramStateRef State,
                     const InvalidatedSymbols *Invalidated,
                     ArrayRef<const MemRegion *> ExplicitRegions,
                     ArrayRef<const MemRegion *> Regions,
                     const LocationContext *LCtx, const CallEvent *Call) const {
    if (isCallbackEnabled(State, "RegionChanges"))
      llvm::errs() << "RegionChanges\n";
    return State;
  }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Registration.
//===----------------------------------------------------------------------===//

void ento::registerAnalysisOrderChecker(CheckerManager &mgr) {
  mgr.registerChecker<AnalysisOrderChecker>();
}

bool ento::shouldRegisterAnalysisOrderChecker(const LangOptions &LO) {
  return true;
}
