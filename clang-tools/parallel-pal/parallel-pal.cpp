/*========================================================================
 * Your Pragmatic Parallel Pal (#pragma pal)
 *========================================================================
 *
 * That friend who's there for you when you wanna talk parallelism!
 *
 * Implements, as a Clang PragmaHandler, a mechanism for developers to
 * suggest optimizations (DOALL, DOACROSS, etc.) by annotating OpenMP
 * structured blocks.
 *
 */

#include "llvm/ADT/StringRef.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
using namespace clang;

namespace {

class ParallelPalPragmaHandler : public PragmaHandler {
public:
  ParallelPalPragmaHandler() : PragmaHandler("pal") { }

  void HandlePragma (
    Preprocessor &PP,
    PragmaIntroducerKind Introducer,
    Token &PragmaTok
  ) override {
    /* TODO(Simone):
     *
     * 1. parse action ("suggest")
     * 2. parse optimization ("doall")
     * 3. make sure the pragma precedes an OpenMP pragma
     * 4. make sure the OpenMP pragma is supported
     * 5. attach annotation to the OpenMP structured block
     *
     */
    Token actionToken;
    PP.LexUnexpandedToken(actionToken);
    if (actionToken.isNot(tok::identifier)) {
      PP.Diag(actionToken, diag::err_pp_expected_after)
        << "'pal'" << "an action identifier";
      return ;
    }

    if (actionToken.getIdentifierInfo()->isStr("suggest")) {
      PP.Diag(actionToken, diag::err_pp_expected_after)
        << "'pal'" << "'suggest'";
      return ;
    }

    Token optimizationToken;
    PP.LexUnexpandedToken(optimizationToken);
    if (optimizationToken.isNot(tok::identifier)) {
      PP.Diag(actionToken, diag::err_pp_expected_after)
        << "'suggest'" << "an optimization identifier";
      return ;
    }

    if (optimizationToken.getIdentifierInfo()->isStr("doall")) {
      PP.Diag(optimizationToken, diag::err_pp_expected_after)
        << "'suggest'" << "'doall'";
      return ;
    }

    /* NOTE(Simone):
     * At this point we've successfully lexed our namespace ('pal'), an
     * action ('suggest'), and the action's argument ('doall').
     *
     * We should expect to be at the end of the declaration (tok::eod).
     * Then, we should expect to see "#pragma omp" for the next 2 tokens
     * ahead of us. Otherwise, we do not know if we have an OpenMP
     * structured block to which to attach our annotation.
     */
    Token endToken;
    PP.LexUnexpandedToken(endToken);
    if (endToken.isNot(tok::eod)) {
      PP.Diag(endToken, diag::ext_pp_extra_tokens_at_eol) << "pragma";
      return ;
    }

    /* NOTE(Simone):
     * Now we need to lex some tokens, but without consuming them. We
     * cannot use `PreProcessor::LookAhead(int n)` because it will skip
     * annotation tokens, and we are trying to verify the existence of
     * OpenMP annotation tokens left behind by the "omp" pragma handler in
     * an earlier phase.
     *
     * So instead we use backtracking. We set the position after the end
     * of our `#pragma pal...` directive as the 'backtrack point', and
     * once we're done looking ahead and verifying that we are indeed
     * followed by a supported OpenMP pragma we will unconditionally
     * backtrack so that the tokens get re-lexed by their actual
     * processing code.
     *
     * IDEA(Simone): what if we *didn't* backtrack? What if we just told
     * the lexer to consume the entire OpenMP annotation? That would
     * probably disable the behavior of the OpenMP pragma entirely, since
     * this phase occurs before codegen and the tokens have already been
     * processed. This might be an avenue for selectively replacing the
     * default behavior of OpenMP pragmas.
     */
    PP.EnableBacktrackAtThisPos();

    Token ompAnnotationToken;
    PP.LexUnexpandedToken(ompAnnotationToken);
    if (ompAnnotationToken.isNot(tok::annot_pragma_openmp)) {
      PP.Diag(ompAnnotationToken, diag::err_pp_expected_after)
        << "'#pragma pal ...'" << "an OpenMP pragma";
      return ;
    }

    Token ompToken;

    PP.LexUnexpandedToken(ompToken); // 'parallel'
    bool isOMPParallel = ompToken.getIdentifierInfo()->isStr("parallel");

    PP.LexUnexpandedToken(ompToken); // annot_pragma_openmp_end
    bool ompDirectiveEnded = ompEndToken.is(tok::annot_pragma_openmp_end);

    if (!(ompDirectiveEnded && isOMPParallel)) {
      PP.Diag(parallelToken, diag::err_pp_expected_after)
        << "'#pragma pal...'" << "'#pragma omp parallel'";
    }

    /**
     * NOTE(Simone): Now we need to annotate the OpenMP structured block
     * with our 'doall' annotation. We can't actually annotate a block of
     * code until we reach the 'Parse' phase. So, for now, we just enter
     * an annotation token here to represent the pragma.
     */

    PP.Backtrack();
  }
};

class ParallelPalFrontendPlugin : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(
    CompilerInstance &CI,
    StringRef
  ) override {
    return llvm::make_unique<ParallelPalASTConsumer>();
  }

  // NOTE(Simone): must be implemented. No args → just return true.
  bool ParseArgs(
    const CompilerInstance &CI,
    const std::vector<std::string> &args
  ) override {
    return true;
  }

  // NOTE(Simone): run automatically after the main AST action.
  /* TODO(Simone): figure out what, exactly, the main AST action is... the
   * documentation on this is sparse - by which I mean, nonexistent. The
   * only reference to a "main AST action" of any kind is in the Clang
   * plugin tutorial.
   */
  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};

class ParallelPalAnnotationVisitor
: public RecursiveASTVisitor<ParallelPalAnnotationVisitor> {
public:
  // need to visit the annotation token for our parallel pal
}

class ParallelPalASTConsumer : public ASTConsumer {
public:
  virtual void HandleTranslationUnit(ASTContext &Context) {
  }
};

}

static PragmaHandlerRegistry::Add<ParallelPalPragmaHandler>
PalPragmaThrowawayInstance("pal","talk to your parallel pal");

static FrontendPluginRegistry::Add<ParallelPalFrontendPlugin>
PalPluginThrowawayInstance("parallel-pal","invite your parallel pal");
