/*========================================================================
 * OMP Hijack - all your OpenMPs are belong to us
 *========================================================================
 *
 * Register an OpenMP PragmaHandler that doesn't emit OpenMP library calls
 * or restructure code. Something something, the harder they fall.
 *
 *------------------------------------------------------------------------
 * BROKEN, DOES NOT WORK, WILL NOT WORK
 *------------------------------------------------------------------------
 *
 * The following code illustrates that we cannot hijack the OpenMP Pragma
 * built-in code. The PragmaHandler registers without error, but will not
 * run. None of the output commands print; the function appears not to run
 * at all. It is safe to assume that this is not an intended use-case for
 * the Clang Plugin Registry. The lack of any errors, warnings, or
 * messages whatsoever is irritating, but not entirely surprising.
 *
 * We must, given this failure, extend Clang's core itself. At that point,
 * it is smarter (and more maintainable) to extend it using our own syntax
 * and constructs, so as to prevent OpenMP's implementation from changing
 * out from under us with future releases of Clang.
 *
 */

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Lex/LexDiagnostic.h"
#include <stdio.h>

using namespace clang;

namespace {

class OpenMPPragmaHandler : public PragmaHandler {
public:
  OpenMPPragmaHandler() : PragmaHandler("omp") { }

  void HandlePragma (
    Preprocessor &PP,
    PragmaIntroducerKind Introducer,
    Token &PragmaTok
  ) override {

    // Test 1: any printf? ✗
    printf("hello?\n");

    // Test 2: any llvm::outs? ✗
    llvm::outs() << "hello???";

    Token parallelToken;
    PP.LexUnexpandedToken(parallelToken);

    if (!parallelToken.getIdentifierInfo()->isStr("parallel")) {
      PP.Diag(parallelToken, diag::err_pp_expected_after)
        << "'omp'" << "'parallel'";
      return ;
    }

    // Test 3: any diagnostics? ✗
    // Now we need to attach metadata to the compound statement
    // (CompoundStmt) which this pragma precedes.
    unsigned DiagID = PP.getDiagnostics().getCustomDiagID(
      DiagnosticsEngine::Warning,
      "Found and handled an OpenMP Pragma"
    );
    PP.Diag(parallelToken.getLocation(), DiagID);
  }
};

}

static PragmaHandlerRegistry::Add<OpenMPPragmaHandler>
ThrowawayInstance("omp","hijack openmp pragmas");
