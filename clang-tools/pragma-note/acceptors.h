/* NOTE(Simone): Acceptors for AST nodes.
 *
 * In our case, we're interested in Decls (e.g. FunctionDecl) and Stmts
 * (e.g. ForStmt, CompoundStmt).
 *
 * The acceptor should be extended to return 'true' only for Decls/Stmts
 * for which codegen has been written and Clang has been extended to call
 * the appropriate codegen handler in note::codegen.
 *
 * EXPLANATION(Simone): This header separates acceptors from the rest of
 * the types and functions implemented as part of the Note annotation
 * system so that Clang headers can import and use acceptors without
 * needing to change the linking configuration for the Clang tree.
 */

#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

namespace note {
  using Decl         = clang::Decl;
  using FunctionDecl = clang::FunctionDecl;

  using Stmt         = clang::Stmt;
  using DoStmt       = clang::DoStmt;
  using ForStmt      = clang::ForStmt;
  using WhileStmt    = clang::WhileStmt;
  using CompoundStmt = clang::CompoundStmt;

  static bool accepts_node (Decl const * node) {
    return false
      || clang::isa<FunctionDecl>(node)
      ;
  }
  static bool accepts_node (Stmt const * node) {
    return false
      || clang::isa<DoStmt>(node)
      || clang::isa<WhileStmt>(node)
      || clang::isa<ForStmt>(node)
      || clang::isa<CompoundStmt>(node)
      ;
  }
}
